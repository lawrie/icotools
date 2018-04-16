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

        input TRIG, ECHO
);

	parameter ECHO_LENGTH = 1;
	parameter TRIG_LENGTH = 1;

	reg[7:0] distance;
	reg valid;
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

		if (done) begin
			req <= 0;
			valid <= 1;
		end

		// Register file:
		//   0x00 data register
		//   0x04 direction register
		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				if (ctrl_addr == 0) begin
					valid <= 0;
					req <= 1;
				end
				ctrl_done <= 1;
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				if (ctrl_addr == 0 && valid) ctrl_rdat <= distance;
			end
		end
	end

	ping p (.clk(clk), .req(req), .echo(echo), .trig(trig), .distance(distance), .done(done));
endmodule

module ping (
  input clk,
  input req,
  input echo,
  output trig,
  output reg [7:0] distance,
  output reg done);

reg [2:0] state = s_idle;
reg [31:0] counter = 0;
reg [15:0] cm_counter;
reg [15:0] cms;
reg trigger;

assign trig = trigger;

localparam s_idle = 3'd0, s_trigger = 3'd1, s_echo_low = 3'd2, 
          s_echo_high = 3'd3, s_pulse = 3'd4, s_done = 3'd5;

always @(posedge clk)
if (req && !done) begin // Request in progress
  if (counter == 200000) begin // If we get to 10 milliseconds, then error
     state <= s_idle;
     done <= 1;
     distance <= 8'd255;
     counter <= 0;
  end
  else case (state)
  s_idle: begin // Got a request, set trigger low for 2 microseconds
       trigger <= 0;
       if (counter == 40) begin
         trigger <= 1;
         counter <= 0;
         state <= s_trigger;
       end
       else counter <= counter + 1;
     end
  s_trigger: begin // Wait for end of trigger pulse, and set it low
       if (counter == 200) begin
         trigger <= 0;
         counter <= 0;
         state <= s_echo_low;
       end
       else counter <= counter + 1;
     end
  s_echo_low: begin // Make sure echo is low
       if (echo == 0) begin
         counter <= 0;
         state <= s_echo_high;
       end
       else counter <= counter + 1;
     end
  s_echo_high: begin // Wait for echo to go high
       if (echo == 1) begin
          counter <= 0;
          cms <= 0;
          state <= s_pulse;
       end 
       else counter <= counter + 1;
     end
  s_pulse: begin // Wait for echo to go low
       if (cm_counter == 1160) begin // cm at 20Mhz
         cm_counter <= 0;
         cms <= cms + 1;
       end
       else cm_counter <= cm_counter + 1;

       if (echo == 0) state <= s_done;
       else counter <= counter + 1;
     end
  s_done: begin // Echo finished, return count in centimeters
       counter <= 0;
       done <= 1;
       distance <= (cms < 255 ? cms : 255);
       state <= s_idle;
     end
  default: begin // Echo finished, return count in centimeters
       counter <= 0; // Don't understand why this default case occurs
       done <= 1;
       distance <= (cms < 255 ? cms : 255);
       state <= s_idle;
     end
  endcase
end
else begin
  state <= s_idle;
  done <= 0;
  counter <= 0;
  trigger <= 0;
end
endmodule

