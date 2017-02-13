module sim_sram (
	input SRAM_A0, SRAM_A1, SRAM_A2, SRAM_A3, SRAM_A4, SRAM_A5, SRAM_A6, SRAM_A7,
	input SRAM_A8, SRAM_A9, SRAM_A10, SRAM_A11, SRAM_A12, SRAM_A13, SRAM_A14, SRAM_A15, SRAM_A16, SRAM_A17, SRAM_A18,
	inout SRAM_D0, SRAM_D1, SRAM_D2, SRAM_D3, SRAM_D4, SRAM_D5, SRAM_D6, SRAM_D7,
	inout SRAM_D8, SRAM_D9, SRAM_D10, SRAM_D11, SRAM_D12, SRAM_D13, SRAM_D14, SRAM_D15,
	input SRAM_CE, SRAM_OE, SRAM_LB, SRAM_UB, SRAM_WE
);
	wire [18:0] sram_addr;
	wire [15:0] sram_din;
	reg [15:0] sram_memory [0:2**19-1];
	reg [15:0] sram_dout;

	assign sram_addr = {SRAM_A18, SRAM_A17, SRAM_A16, SRAM_A15, SRAM_A14, SRAM_A13, SRAM_A12, SRAM_A11, SRAM_A10, SRAM_A9, SRAM_A8,
			SRAM_A7, SRAM_A6, SRAM_A5, SRAM_A4, SRAM_A3, SRAM_A2, SRAM_A1, SRAM_A0};

	assign sram_din = {SRAM_D15, SRAM_D14, SRAM_D13, SRAM_D12, SRAM_D11, SRAM_D10, SRAM_D9, SRAM_D8,
			SRAM_D7, SRAM_D6, SRAM_D5, SRAM_D4, SRAM_D3, SRAM_D2, SRAM_D1, SRAM_D0};

	assign {SRAM_D15, SRAM_D14, SRAM_D13, SRAM_D12, SRAM_D11, SRAM_D10, SRAM_D9, SRAM_D8,
			SRAM_D7, SRAM_D6, SRAM_D5, SRAM_D4, SRAM_D3, SRAM_D2, SRAM_D1, SRAM_D0} = sram_dout;

	always @(SRAM_WE, SRAM_CE, SRAM_OE, SRAM_LB, SRAM_UB, sram_addr, sram_din) begin
		// Truth table on page 2 of the SRAM data sheet:
		// http://www.issi.com/WW/pdf/61-64WV6416DAxx-DBxx.pdf

		sram_dout = 16'bz;
		casez ({SRAM_WE, SRAM_CE, SRAM_OE, SRAM_LB, SRAM_UB})
			// Not Selected / Output Disabled
			5'b z1zzz, 5'b 101zz, 5'b z0z11:;
			// Read
			5'b 10001: sram_dout = {8'bz, sram_memory[sram_addr][7:0]};
			5'b 10010: sram_dout = {sram_memory[sram_addr][15:8], 8'bz};
			5'b 10000: sram_dout = sram_memory[sram_addr];
			// Write
			5'b 00z01: sram_memory[sram_addr][7:0] = sram_din[7:0];
			5'b 00z10: sram_memory[sram_addr][15:8] = sram_din[15:8];
			5'b 00z00: sram_memory[sram_addr] = sram_din;
		endcase
	end
endmodule
