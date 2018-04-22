module icosoc_mod_vga_text (
	input clk,
	input resetn,

	input [ 3:0] ctrl_wr,
	input        ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,
	input clk100,

	inout [15:0] VGA
);
	parameter integer CLOCK_FREQ_HZ = 6000000;
	parameter VGA_LENGTH = 16;

	reg [15:0] vga;
	wire clk_100;

	SB_IO #(
		.PIN_TYPE(6'b 0110_01),
		.PULLUP(1'b 0)
	) vga_output [15:0] (
		.PACKAGE_PIN(VGA),
		.D_OUT_0(vga)
	);

	reg[7:0] x, y, char;
	wire we;
	wire [7:0] data_in, data_out;
	reg [15:0] r_addr, w_addr;
	reg [7:0] ascii;

	always @(posedge clk100) begin
		r_addr <= ((yb >> 4) << 6) + (xb >> 3);
		if (data_out != 0) ascii <= data_out;
	end

	always @(posedge clk) begin
		ctrl_done <= 0;
		ctrl_rdat <= 'bx;
		we <= 0;

		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;			
				if (ctrl_addr == 0) begin
					w_addr <= ctrl_wdat[23:8];
					data_in <= ctrl_wdat[7:0];
					we <= 1;
				end
			end

		end
	end

        wire [9:0] xb, yb;
	wire pixel;
	wire show_text = data_out > 0 && pixel;

        assign vga[3:0] = show_text?15:0; // red
	assign vga[7:4] = show_text?15:0; // blue
	assign vga[11:8] = show_text?15:0; // green
	assign vga[14] = 0;
	assign vga[15] = 0;

	vga vga0 (.CLK(clk100), .HS(vga[12]), .VS(vga[13]), .x(xb), .y(yb));

	vga_buff vb (.clk(clk100), .we(we), .r_addr(r_addr), .w_addr(w_addr),
		     .data_in(data_in), .data_out(data_out));

	font8x16 f (.ascii_code(ascii), .row(yb[3:0]), .col(xb[2:0]), 
                       .pixel(pixel));
endmodule

module vga(
    input CLK,  // 100 Mhz
    output HS, VS,
    output [9:0] x,
    output [9:0] y,
    output blank
    );

reg [9:0] xc;
reg [15:0] prescaler = 0;
 
// Horizontal 640 + fp 24 + HS 40 + bp 128 = 832 pixel clocks
// Vertical, 480 + fp 9 lines vs 3 lines bp 28 lines 
assign blank = ((xc < 192) | (xc > 832) | (y > 479));
assign HS = ~ ((xc > 23) & (xc < 65));
assign VS = ~ ((y > 489) & (y < 493));
assign x = ((xc < 192)?0:(xc - 192));

always @(posedge CLK)
begin
  prescaler <= prescaler + 1;
  if (prescaler == 3)
  begin
    prescaler <= 0;
    if (xc == 832)
    begin
      xc <= 0;
      y <= y + 1;
    end
    else begin
      xc <= xc + 1;
    end
    if (y == 520)
    begin
      y <= 0; 
    end
  end
end

endmodule

module vga_buff(
  input clk,
  input we,
  input [7:0] data_in,
  input [15:0] w_addr,
  input [15:0] r_addr,
  output reg [7:0] data_out);

reg [7:0] mem [0:1919];

always @(posedge clk) begin
  if (we && w_addr < 1920) mem[w_addr] <= data_in;
  data_out <= mem[r_addr];
end

endmodule

module font8x16 (
		 input  [7:0]	ascii_code,
		 input  [3:0]	row,
		 input  [2:0]   col,
		 output pixel
		 );

reg [127:0] fnt [0:255];
initial $readmemh("../../mod_vga_text/font.hex", fnt);

wire [7:0] r = row;

assign pixel = fnt[ascii_code][(r << 3) + ~col];
	 
endmodule
