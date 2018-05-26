module icosoc_mod_pullup #(
	parameter integer CLOCK_FREQ_HZ = 0,
	parameter integer IO_LENGTH = 32
) (
	input clk,
	input resetn,

	input [3:0] ctrl_wr,
	input ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	inout [IO_LENGTH-1:0] IO
);
	wire [IO_LENGTH-1:0] io_in;
	reg [IO_LENGTH-1:0] io_out;
	reg [IO_LENGTH-1:0] io_dir;

	SB_IO #(
		.PIN_TYPE(6'b 1010_01),
		.PULLUP(1'b 1)
	) ios [IO_LENGTH-1:0] (
		.PACKAGE_PIN(IO),
		.OUTPUT_ENABLE(io_dir),
		.D_OUT_0(io_out),
		.D_IN_0(io_in)
	);

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		// Register file:
		//   0x00 data register
		//   0x04 direction register
		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0) io_out <= ctrl_wdat;
				if (ctrl_addr == 4) io_dir <= ctrl_wdat;
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0) ctrl_rdat <= io_in;
				if (ctrl_addr == 4) ctrl_rdat <= io_dir;
			end
		end
	end
endmodule

