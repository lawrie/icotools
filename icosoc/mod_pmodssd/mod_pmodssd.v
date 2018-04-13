// Description of the Pmod SSD:
// https://reference.digilentinc.com/reference/pmod/pmodssd/reference-manual

module icosoc_mod_pmodssd #(
	parameter integer CLOCK_FREQ_HZ = 6000000
) (
	input clk,
	input resetn,

	input [3:0] ctrl_wr,
	input ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	output reg [15:0] pins
);
	reg [7:0] value;

	function [6:0] segments;
		input [3:0] x;
		case (x)
			4'h0: segments = 7'b1111110;
			4'h1: segments = 7'b0110000;
			4'h2: segments = 7'b1101101;
			4'h3: segments = 7'b1111001;
			4'h4: segments = 7'b0110011;
			4'h5: segments = 7'b1011011;
			4'h6: segments = 7'b1011111;
			4'h7: segments = 7'b1110000;
			4'h8: segments = 7'b1111111;
			4'h9: segments = 7'b1111011;
			4'hA: segments = 7'b1110111;
			4'hB: segments = 7'b0011111;
			4'hC: segments = 7'b1001110;
			4'hD: segments = 7'b0111101;
			4'hE: segments = 7'b1001111;
			4'hF: segments = 7'b1000111;
		endcase
	endfunction

	wire [6:0] segs = segments(digit_select ? value[7:4] : value[3:0]);

        reg digit_select;
	always @(posedge clk) begin
		digit_select <= !digit_select;
		pins <= {segs[3], segs[4], segs[5], segs[6], 4'b0000,
			digit_select, segs[0], segs[1], segs[2], 4'b0000};
	end

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;
				value <= ctrl_wdat;
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				ctrl_rdat <= value;
			end
		end
	end


endmodule
