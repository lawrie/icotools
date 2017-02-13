module sim_hram (
	input HRAM_CS, HRAM_CK,
	inout HRAM_RWDS, HRAM_DQ0, HRAM_DQ1, HRAM_DQ2, HRAM_DQ3, HRAM_DQ4, HRAM_DQ5, HRAM_DQ6, HRAM_DQ7
);
	reg hram_dir = 0;
	reg [7:0] hram_dout = 0;
	wire [7:0] hram_din = {HRAM_DQ7, HRAM_DQ6, HRAM_DQ5, HRAM_DQ4, HRAM_DQ3, HRAM_DQ2, HRAM_DQ1, HRAM_DQ0};
	assign {HRAM_DQ7, HRAM_DQ6, HRAM_DQ5, HRAM_DQ4, HRAM_DQ3, HRAM_DQ2, HRAM_DQ1, HRAM_DQ0} = hram_dir ? hram_dout : 8'bz;

	integer state = 0;
	reg [47:0] command;
	reg [7:0] hibyte;
	reg hibyte_en;

	wire cmd_rd = command[47];
	wire cmd_reg = command[46];
	wire cmd_lin = command[45];
	wire [31:0] cmd_addr = {command[44:16], command[2:0]};

	reg long_latency = 0;
	reg drive_rwds_hi = 0;
	reg drive_rwds_lo = 0;

	assign HRAM_RWDS = HRAM_CS ? 1'bz :
		state < 6 ? long_latency :
		drive_rwds_hi ? 1'b1 :
		drive_rwds_lo ? 1'b0 : 1'bz;

	reg [15:0] memory [0:65535];

	always @(posedge HRAM_CS, posedge HRAM_CK, negedge HRAM_CK) begin
		if (HRAM_CS) begin
			state <= 0;
			hram_dir <= 0;
			hram_dout <= 0;
			drive_rwds_hi <= 0;
			drive_rwds_lo <= 0;
			long_latency <= $random & 1;
		end else begin
			state <= state + 1;

			if (state < 6) begin
				command <= {command, hram_din};
			end else begin
				if (cmd_rd) begin
					if (state == 6 && cmd_reg) begin
						// ???
					end
					if (state == 7 && cmd_reg) begin
						$display("HRAM REGISTER READ @%x: %x", cmd_addr, {hibyte, hram_din});
						// ???
					end

					if (state == (long_latency ? 20 : 12)) begin
						hram_dir <= 1;
						hram_dout <= memory[cmd_addr[15:0]][15:8];
						drive_rwds_hi <= 1;
						drive_rwds_lo <= 0;
					end
					if (state == (long_latency ? 21 : 13)) begin
						$display("HRAM MEMORY READ @%x: %x", cmd_addr, memory[cmd_addr[15:0]]);
						hram_dir <= 1;
						hram_dout <= memory[cmd_addr[15:0]][7:0];
						drive_rwds_hi <= 0;
						drive_rwds_lo <= 1;
						state <= state - 1;
					end
				end else begin
					if (state == 6 && cmd_reg) begin
						hibyte <= hram_din;
					end
					if (state == 7 && cmd_reg) begin
						$display("HRAM REGISTER WRITE @%x: %x", cmd_addr, {hibyte, hram_din});
					end

					if (state == (long_latency ? 20 : 12)) begin
						hibyte <= hram_din;
						hibyte_en <= HRAM_RWDS;
					end
					if (state == (long_latency ? 21 : 13)) begin
						$display("HRAM MEMORY WRITE @%x: %x [%b]", cmd_addr, {hibyte, hram_din}, {hibyte_en, HRAM_RWDS});
						if (hibyte_en) memory[cmd_addr[15:0]][15:8] <= hibyte;
						if (HRAM_RWDS) memory[cmd_addr[15:0]][7:0] <= hram_din;
						{command[44:16], command[2:0]} <= {command[44:16], command[2:0]} + 1;
						state <= state - 1;
					end
				end
			end
		end
	end
endmodule
