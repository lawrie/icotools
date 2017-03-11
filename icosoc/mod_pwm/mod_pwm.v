module icosoc_mod_pwm (
	input clk,
	input resetn,

	input [ 3:0] ctrl_wr,
	input        ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	output reg pin
);
	parameter integer CLOCK_FREQ_HZ = 6000000;

	reg [31:0] counter;
	reg [31:0] max_cnt, on_cnt, off_cnt;

	always @(posedge clk) begin
		ctrl_done <= 0;
		ctrl_rdat <= 'bx;

		counter <= counter < max_cnt ? counter + 1 : 0;
		if (counter == on_cnt) pin <= 1;
		if (counter == off_cnt) pin <= 0;

		if (!resetn) begin
			counter <= 0;
			max_cnt <= 0;
			on_cnt <= 0;
			off_cnt <= 0;
			pin <= 0;
		end else
		if (!ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;
				case (ctrl_addr)
					15'h 0000: counter <= ctrl_wdat;
					15'h 0004: max_cnt <= ctrl_wdat;
					15'h 0008: on_cnt <= ctrl_wdat;
					15'h 000c: off_cnt <= ctrl_wdat;
				endcase
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				case (ctrl_addr)
					15'h 0000: ctrl_rdat <= counter;
					15'h 0004: ctrl_rdat <= max_cnt;
					15'h 0008: ctrl_rdat <= on_cnt;
					15'h 000c: ctrl_rdat <= off_cnt;
				endcase
			end
		end
	end
endmodule
