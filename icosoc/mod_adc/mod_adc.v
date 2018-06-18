module icosoc_mod_adc #(
	parameter integer CLOCK_FREQ_HZ = 0
) (
	input clk,
	input resetn,

	input [3:0] ctrl_wr,
	input ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	inout [7:0] data,
	inout ad_clk
);
	wire [7:0] data_in;
	wire ad_clk_out;

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) data_input [7:0] (
		.PACKAGE_PIN(data),
		.D_IN_0(data_in)
	);


	SB_IO #(
		.PIN_TYPE(6'b 0110_01),
		.PULLUP(1'b 0)
	) ad_clk_output (
		.PACKAGE_PIN(ad_clk),
		.D_OUT_0(ad_clk_out)
	);

	assign ad_clk_out = clk;

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		if (resetn && !ctrl_done) begin
			if (ctrl_rd) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0) ctrl_rdat <= data_in;
			end
		end
	end
endmodule

