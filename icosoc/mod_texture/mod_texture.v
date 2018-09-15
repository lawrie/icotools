module icosoc_mod_texture (
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
	reg [9:0] sprite_x = 20, sprite_y = 30;

	SB_IO #(
		.PIN_TYPE(6'b 0110_01),
		.PULLUP(1'b 0)
	) vga_output [15:0] (
		.PACKAGE_PIN(VGA),
		.D_OUT_0(vga)
	);

	wire px_clk;
        SB_PLL40_CORE #(
                .FEEDBACK_PATH("SIMPLE"),
                .DIVR(4'b1001),         // DIVR =  9
                .DIVF(7'b1100100),      // DIVF = 100
                .DIVQ(3'b101),          // DIVQ =  5
                .FILTER_RANGE(3'b001)   // FILTER_RANGE = 1
        ) uut (
                .RESETB(1'b1),
                .BYPASS(1'b0),
                .REFERENCECLK(clk100),
                .PLLOUTCORE(px_clk)
                );

	wire we;
	wire [7:0] data_in; 
	
	always @(posedge clk100) begin
	end

	always @(posedge clk) begin
		ctrl_done <= 0;
		ctrl_rdat <= 'bx;
		we <= 0;

		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;			
				if (ctrl_addr == 0) begin
					data_in <= ctrl_wdat[7:0];
					we <= 1;
				end else if (ctrl_addr == 4) begin
					sprite_y <= ctrl_wdat[25:16];
					sprite_x <= ctrl_wdat[9:0];
				end
			end

		end
	end

        wire [9:0] xb, yb;
	wire blank;
        wire on_sprite = (xb >= sprite_x && xb < sprite_x + 32 &&
                          yb >= sprite_y && yb < sprite_y + 32);

        assign vga[3:0] = ((on_sprite ? sprite_rgb[2] : rom_rgb[0]) & !blank)?15:0; // red
	assign vga[7:4] = ((on_sprite ? sprite_rgb[1] : rom_rgb[1]) & !blank)?15:0; // blue
	assign vga[11:8] = ((on_sprite ? sprite_rgb[0] : rom_rgb[2]) & !blank)?15:0; // green
	assign vga[14] = 0;
	assign vga[15] = 0;

	vga vga0 (.CLK(clk100), .HS(vga[12]), .VS(vga[13]), .x(xb), .y(yb), .blank(blank));

    wire [5:0] texture_idx;
    map_rom map(.clk(clk100), .x_idx(xb[8:3]), .y_idx(yb[8:3]), .val(texture_idx));

    reg [2:0] rom_rgb;
    texture_rom texture(.clk(clk100), .texture_idx(texture_idx), .y_idx(yb[2:0]), .x_idx(xb[2:0]), .val(rom_rgb));

    reg[2:0] sprite_rgb;
    sprite_rom sprite(.clk(clk100), .y_idx(yb[4:0] - sprite_y[4:0]), .x_idx(xb[4:0] - sprite_x[4:0]), .rgb(sprite_rgb)); 

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
assign x = ((xc < 192)?0:(xc-192));

always @(posedge CLK) begin
  prescaler <= prescaler + 1;
  if (prescaler == 3)
  begin
    prescaler <= 0;
    if (xc == 832) begin
      xc <= 0;
      y <= y + 1;
    end else begin 
      xc <= xc + 1;
    end
    if (y == 520) begin
      y <= 0; 
    end
  end
end

endmodule

module texture_rom(
  input clk,
  input [5:0] texture_idx,
  input [2:0] y_idx,
  input [2:0] x_idx,
  output reg [2:0] val);

  reg[2:0] TEXTURE_ROM[0:4095];  /* 16 x (32 x 32 x 4bpp) textures */
  initial $readmemh ("textures.mem", TEXTURE_ROM);

  // the indexing below is a little bit complicated, but is because our
  // texture image is 64x64 pixels, split into 8x8 textures.  I'm fairly
  // confident the math works out. :D
  always @(posedge clk) begin
    val <= TEXTURE_ROM[{ texture_idx[5:3], y_idx, texture_idx[2:0], x_idx }];
  end

endmodule

module map_rom(
  input clk,
  input [5:0] y_idx, /* 0..63 */
  input [5:0] x_idx, /* 0..63 */
  output reg [5:0] val);

  reg[5:0] TILEMAP_ROM[0:4096];  /* 64x64 x 6-bits per tile */
  initial $readmemh ("tile_map.mem", TILEMAP_ROM);

  always @(posedge clk) begin
    val <= TILEMAP_ROM[{y_idx, x_idx}];
  end

endmodule

module sprite_rom(
  input clk,
  input [4:0] y_idx,
  input [4:0] x_idx,
  output reg [2:0] rgb);

  reg[2:0] SPRITE_ROM[0:1023]; 
  initial $readmemh ("mario.mem", SPRITE_ROM);

  always @(posedge clk) begin
    rgb <= SPRITE_ROM[{ y_idx,  x_idx }];
  end

endmodule
