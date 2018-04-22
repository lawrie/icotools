module icosoc_mod_tone (
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

	reg [31:0] period;

	always @(posedge clk) begin
		ctrl_done <= 0;
		ctrl_rdat <= 'bx;

		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;			
				if (ctrl_addr == 0) period <= ctrl_wdat;
				
			end			
		end
	end

	tone t (.CLK(clk), .period(period), .tone_out(pin_out));
endmodule

module tone(
    input CLK,
    input[31:0] period, // microseconds 
    output reg tone_out
    );

parameter CLK_F = 20; // CLK freq in MHz

reg [7:0] prescaler = 0; 
reg [15:0] counter = 0;

always @(posedge CLK)
begin
  prescaler <= prescaler + 1;
  if (prescaler == CLK_F / 2 - 1) 
  begin
    prescaler <= 0;
    counter <= counter + 1;
    if (counter == period - 1)
    begin
      counter <= 0;
      tone_out <= ~ tone_out;
    end		
  end  
end

endmodule
