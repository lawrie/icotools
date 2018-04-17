module icosoc_mod_ps2 #(
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

        inout PS2_CLK, PS2_DAT
);

	parameter PS2_CLK_LENGTH = 1;
	parameter PS2_DAT_LENGTH = 1;

	reg[7:0] scan_code;
	reg ps2_dat, ps2_clk;
	wire valid, error;
	reg got_code = 0;

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) ps2_clk_input (
		.PACKAGE_PIN(PS2_CLK),
		.D_IN_0(ps2_clk)
	);

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 0)
	) ps2_dat_input (
		.PACKAGE_PIN(PS2_DAT),
		.D_IN_0(ps2_dat)
	);

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		if (valid) got_code <= 1;


		// Register file:
		//   0x00 data register
		//   0x04 direction register
		if (resetn && !ctrl_done) begin
			if (ctrl_rd) begin
				if (got_code) begin
					ctrl_rdat <= scan_code;
					got_code <= 0;
                                end
				else ctrl_rdat <= 32'hffffffff;

				ctrl_done <= 1;
			end
		end
	end

	ps2_intf ps2 (.CLK(clk), .nRESET(resetn), .PS2_CLK(ps2_clk), .PS2_DATA(ps2_dat),
                      .DATA(scan_code), .VALID(valid), .error(error));

endmodule

// This is input-only for the time being
module ps2_intf
  (
   input            CLK,
   input            nRESET,
                 
   // PS/2 interface (could be bi-dir)
   input            PS2_CLK,
   input            PS2_DATA,
   
   // Byte-wide data interface - only valid for one clock
   // so must be latched externally if required
   output reg [7:0] DATA,
   output reg       VALID,
   output reg       error
   );
   
   reg [7:0]        clk_filter;
   reg              ps2_clk_in;
   reg              ps2_dat_in;
   reg              clk_edge; // Goes high when a clock falling edge is detected
   reg [3:0]        bit_count;
   reg [8:0]        shiftreg;
   reg              parity;


   always @(posedge CLK, negedge nRESET)
     begin
        if (!nRESET)
          begin
             ps2_clk_in <= 1'b1;
             ps2_dat_in <= 1'b1;
             clk_filter <= 8'hff;
             clk_edge   <= 1'b0;
             end
        else
          begin
            // Register inputs (and filter clock)
             ps2_dat_in <= PS2_DATA;
             clk_filter <= { PS2_CLK, clk_filter[7:1] };
             clk_edge   <= 1'b0;

             if (clk_filter == 8'hff)
               // Filtered clock is high
               ps2_clk_in <= 1'b1;
             else if (clk_filter == 8'h00)
               begin
                  // Filter clock is low, check for edge
                  if (ps2_clk_in)
                    clk_edge <= 1'b1;
                  ps2_clk_in <= 1'b0;
               end
          end        
     end

    // Shift in keyboard data
   always @(posedge CLK, negedge nRESET)
     begin
        if (!nRESET)
          begin
             bit_count <= 0;
             shiftreg  <= 0;
             DATA      <= 0;             
             parity    <= 1'b0;
             VALID     <= 1'b0;
             error     <= 1'b0;
          end
        else
          begin
             // Clear flags
             VALID <= 1'b0;
             error <= 1'b0;

             if (clk_edge)
               // We have a new bit from the keyboard for processing
               if (bit_count == 0)
                 begin
                    // Idle state, check for start bit (0) only and don't
                    // start counting bits until we get it
                    
                    parity <= 1'b0;
                    if (!ps2_dat_in)                                        
                      bit_count <= bit_count + 1; // This is a start bit
                 end
               else
                 begin                 
                    // Running.  8-bit data comes in LSb first followed by
                    // a single stop bit (1)
                    if (bit_count < 10)
                      begin
                         // Shift in data and parity (9 bits)
                         bit_count <= bit_count + 1;
                         shiftreg  <= { ps2_dat_in,  shiftreg[8:1] };
                         parity    <= parity ^ ps2_dat_in;
                      end
                    else if (ps2_dat_in)
                      begin
                         // Valid stop bit received
                         bit_count <= 0;        // back to idle
                         if (parity)
                           begin
                              // Parity correct, submit data to host
                              DATA  <= shiftreg[7:0];
                              VALID <= 1'b1;
                           end
                         else
                           error <= 1'b1;
                      end
                    else
                      begin
                         // Invalid stop bit
                         bit_count <= 0;        // back to idle
                         error     <= 1'b1;
                      end
                 end
          end
     end
endmodule
