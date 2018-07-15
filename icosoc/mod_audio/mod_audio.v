module icosoc_mod_audio (
	input clk,
	input resetn,

	input [ 3:0] ctrl_wr,
	input        ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	inout pin
);

	parameter integer CLOCK_FREQ_HZ = 20000000;
	reg pin_out;

	SB_IO #(
		.PIN_TYPE(6'b 0110_01),
		.PULLUP(1'b 0)
	) pin_io (
		.PACKAGE_PIN(pin),
		.D_OUT_0(pin_out)
	);

	reg [7:0] data;
	wire data_ready;

	always @(posedge clk) begin
		ctrl_done <= 0;
		ctrl_rdat <= 'bx;
		data_ready <= 0;

		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;			
				if (ctrl_addr == 0) begin
					data <= ctrl_wdat[7:0];
					data_ready <= 1;
				end
			end			
		end
	end

	PWM p1 (.clk(clk), .RxD_data(data), .RxD_data_ready(data_ready), 
               .PWM_out(pin_out));
endmodule

module PWM(input clk, input RxD_data_ready, input [7:0] RxD_data, output PWM_out);

reg [7:0] RxD_data_reg;
always @(posedge clk) if(RxD_data_ready) RxD_data_reg <= RxD_data;
////////////////////////////////////////////////////////////////////////////
reg [8:0] PWM_accumulator;
always @(posedge clk) PWM_accumulator <= PWM_accumulator[7:0] + RxD_data_reg;

assign PWM_out = PWM_accumulator[8];
endmodule
