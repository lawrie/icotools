module icosoc_mod_rotary #(
	parameter integer CLOCK_FREQ_HZ = 0,
        parameter integer QA_LENGTH = 1,
	parameter integer QB_LENGTH = 1
) (
	input clk,
	input resetn,

	input [3:0] ctrl_wr,
	input ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	output [31:0] COUNT,
        input QA, QB
);
	wire [31:0] count;
        wire qa, qb;

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) qa_input (
		.PACKAGE_PIN(QA),
		.D_IN_0(qa)
	);

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) qb_input (
		.PACKAGE_PIN(QB),
		.D_IN_0(qb)
	);

        quad q (.clk(clk), .quadA(qa), .quadB(qb), .count(count));

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		// Register file:
		//   0x00 data register
		//   0x04 direction register
		if (resetn && !ctrl_done) begin
			if (ctrl_rd) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0) ctrl_rdat <= count;
				if (ctrl_addr == 4) ctrl_rdat <= 0;
			end
		end
	end
endmodule

module quad(clk, quadA, quadB, count);
input clk, quadA, quadB;
output reg [31:0] count;

reg [2:0] quadA_delayed, quadB_delayed;
always @(posedge clk) quadA_delayed <= {quadA_delayed[1:0], quadA};
always @(posedge clk) quadB_delayed <= {quadB_delayed[1:0], quadB};

wire count_enable = quadA_delayed[1] ^ quadA_delayed[2] ^ quadB_delayed[1] ^ quadB_delayed[2];
wire count_direction = quadA_delayed[1] ^ quadB_delayed[2];

always @(posedge clk)
begin
  if(count_enable)
  begin
    if(count_direction) count<=count+1; else count<=count-1;
  end
end

endmodule

