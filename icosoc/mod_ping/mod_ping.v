module icosoc_mod_ping #(
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

	output [31:0] COUNT,
        input TRIG, ECHO
);

	parameter ECHO_LENGTH = 1;
	parameter TRIG_LENGTH = 1;

	wire [31:0] count;
        wire echo, trig, req, done;

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) echo_input (
		.PACKAGE_PIN(ECHO),
		.D_IN_0(echo)
	);

	SB_IO #(
		.PIN_TYPE(6'b 0110_01),
		.PULLUP(1'b 0)
	) trigger_output (
		.PACKAGE_PIN(TRIG),
		.D_OUT_0(trig)
	);

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		if (done) req <= 0;

		// Register file:
		//   0x00 data register
		//   0x04 direction register
		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				if (ctrl_addr == 0) begin
					req <= 1;
				end
				ctrl_done <= 1;
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0 && done) ctrl_rdat <= count;
			end
		end
	end

	ping p (.clk(clk), .req(req), .echo(echo), .trig(trig), .distance(count), .done(done));
endmodule

module ping (
  input clk,
  input req,
  input echo,
  output trig,
  output reg [7:0] distance,
  output reg done);

reg [2:0] state = 3'd0;
reg [31:0] counter;
reg [15:0] cm_counter;
reg [7:0] cms;
reg trigger;

assign trig = trigger;

always @(posedge clk)
if (req && !done) begin // Request in progress
  if (counter == 1000000) begin // If we get to 10 milliseconds, then error
     state <= 3'd0;
     done <= 1;
     cms <= 8'd255;
     counter <= 0;
     trigger <= 0;
  end
  else case (state)
  3'd0: begin // Got a request, set trigger low for 2 microseconds
       trigger <= 0;
       if (counter == 200) begin
         trigger <= 1;
         counter <= 0;
         state <= 3'd1;
       end
       else counter <= counter + 1;
     end
  3'd1: begin // Wait for end of trigger pulse, and set it low
       if (counter == 1000) begin
         trigger <= 0;
         counter <= 0;
         state <= 3'd2;
       end
       else counter <= counter + 1;
     end
  3'd2: begin // Make sure echo is low
       if (echo == 0) begin
         counter <= 0;
         state <= 3'd3;
       end
       else counter <= counter + 1;
     end
  3'd3: begin // Wait for echo to go high
       if (echo == 1) begin
          counter <= 0;
          cms <= 0;
          state <= 3'd4;
       end 
       else counter <= counter + 1;
     end
  3'd4: begin // Wait for echo to go low
       if (cm_counter == 1160) begin // cm at 20Mhz
         cm_counter <= 0;
         cms <= cms + 1;
       end
       else cm_counter <= cm_counter + 1;

       if (echo == 0) state <= 3'd5;
       else counter <= counter + 1;
     end
  3'd5: begin // Echo finished, return count in centimeters
       counter <= 0;
       done <= 1;
       distance = cms;
       state <= 3'd0;
     end
  default: begin // Echo finished, return count in centimeters
       counter <= 0; // Don't understand why this occurs
       done <= 1;
       distance = cms;
       state <= 3'd0;
     end
  endcase
end
else begin
  state <= 3'd0;
  done <= 0;
  counter <= 0;
  trigger <= 0;
end
endmodule

