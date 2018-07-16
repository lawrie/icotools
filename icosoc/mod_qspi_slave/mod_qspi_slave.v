module icosoc_mod_qspi_slave #(
	parameter integer CLOCK_FREQ_HZ = 0,
	parameter integer CS_LENGTH = 32
) (
	input clk,
	input resetn,

	input [3:0] ctrl_wr,
	input ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	inout [3:0] qd,
	inout qck,
	inout qss
);
	wire qck_in, qss_in;
	wire [7:0] tx_data, rx_data;
	wire [23:0] count;
	wire [3:0] qd_in;
	reg [3:0] qd_dir;
	reg [3:0] qd_out;

	SB_IO #(
		.PIN_TYPE(6'b 1010_01),
		.PULLUP(1'b 0)
	) io_qd [3:0] (
		.PACKAGE_PIN(qd),
		.OUTPUT_ENABLE(qd_dir),
		.D_OUT_0(qd_out),
		.D_IN_0(qd_in)
	);

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) io_qck (
		.PACKAGE_PIN(qck),
		.D_IN_0(qck_in)
	);

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) io_qss (
		.PACKAGE_PIN(qss),
		.D_IN_0(qss_in)
	);

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;
		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0) tx_data <= ctrl_wdat[7:0];
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0) begin
					ctrl_rdat = {count, rx_data};
					count <= 0;
				end
			end
		end
		
		// state machine to alternate reading and writing
		case (writing)
		0: begin
			// receive byte of data
			if (spi_rxready) begin
				rx_data <= spi_rxdata;
				count <= count + 1;
			end
			// when chip select rises, switch to writing state
			if (deselect) begin
				spi_txdata <= tx_data;
				qd_dir <= 4'b1111;
				writing <= 1;
			end
	   	end
		1: begin
			if (deselect) begin
				qd_dir <= 4'b0000;
				writing <= 0;
			end
	   	end
		endcase
	end

	reg writing;
	reg [7:0] spi_txdata, spi_rxdata;
	wire spi_txready, spi_rxready;

	// synchronise chip select signal, to switch from reading to writing
	reg [2:0] select;
	always @(posedge clk)
		select <= {select[1:0],~qss_in};
	wire deselect = (select[1:0] == 2'b10);

	qspislave_tx #(.DWIDTH(4)) QT (
		.clk(clk),
		.txdata(spi_txdata),
		.txready(spi_txready),
		.QCK(qck_in),
		.QSS(qss_in),
		.QD(qd_out)
	);

	qspislave_rx #(.DWIDTH(4)) QR (
		.clk(clk),
		.rxdata(spi_rxdata),
		.rxready(spi_rxready),
		.QCK(qck_in),
		.QSS(qss_in),
		.QD(qd_in)
	);
endmodule

// Receive QSPI data from master to slave
// DWIDTH (1 or 4) is number of data lines
// rxready is asserted for one clk period
//   when each input byte is available in rxdata

module qspislave_rx #(parameter DWIDTH=1) (
	input clk,
	input QCK, QSS,
	input [3:0] QD,
	output rxready,
	output [7:0] rxdata
);

	// registers in QCK clock domain
	reg [8:0] shiftreg;
	reg inseq;

	// registers in main clk domain
	reg [7:0] inbuf;
	assign rxdata = inbuf;
	reg [2:0] insync;

	// synchronise inseq across clock domains
	always @(posedge clk)
		insync <= {inseq,insync[2:1]};
	assign rxready = (insync[1] != insync[0]);

	// wiring to load data from 1 or 4 data lines into shiftreg
	wire [8:0] shiftin = {shiftreg[8-DWIDTH:0],QD[DWIDTH-1:0]};

	// capture incoming data on rising SPI clock edge
	always @(posedge QCK or posedge QSS)
		if (QSS)
			shiftreg <= 0;
		else begin
			if (shiftin[8]) begin
				inbuf <= shiftin[7:0];
				inseq <= ~inseq;
				shiftreg <= 0;
			end else if (shiftreg[7:0] == 0)
				shiftreg = {1'b1,QD[DWIDTH-1:0]};
			else
				shiftreg <= shiftin;
		end

endmodule

// Transmit QSPI data from slave to master
// txready is asserted for one clk period
//   when one output byte has been sent from txdata,
//   and the next byte must be supplied before the next
//   rising edge of QCK

module qspislave_tx #(parameter DWIDTH=1) (
	input clk,
	input QCK, QSS,
	output [3:0] QD,
	output txready,
	input [7:0] txdata
);

	// registers in QCK clock domain
	reg [8:0] shiftreg;
	reg outseq;
	assign QD[3:0] = shiftreg[8:9-DWIDTH];

	// registers in main clk domain
	reg [2:0] outsync;

	// synchronise outseq across clock domains
	always @(posedge clk)
		outsync <= {outseq,outsync[2:1]};
	assign txready = (outsync[1] != outsync[0]);

	// wiring to shift data from shiftreg into 1 or 4 data lines
	wire [8:0] shiftout = shiftreg << DWIDTH;

	// shift outgoing data on falling SPI clock edge
	always @(negedge QCK or posedge QSS)
		if (QSS)
			shiftreg <= 0;
		else begin
			if (shiftout[7:0] == 0) begin
				outseq <= ~outseq;
				shiftreg <= {txdata,1'b1};
			end else
				shiftreg <= shiftout;
		end

endmodule

