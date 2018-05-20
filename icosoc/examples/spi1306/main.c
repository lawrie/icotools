#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"
#include "gfx.h"

void send_cmd(uint8_t r) {
	icosoc_oled_cs(1);
	icosoc_oled_dc(0);
	icosoc_oled_cs(0);

	uint8_t d = icosoc_oled_xfer(r);
	icosoc_oled_cs(1);
	//printf("Command %x\n", d);
}

void send_data(uint8_t d) {
	icosoc_oled_cs(1);
	icosoc_oled_dc(1);
	icosoc_oled_cs(0);

	uint8_t r2 = icosoc_oled_xfer(d);

	icosoc_oled_cs(1);
	//printf("Data %x\n", r);
}

void display() {
	//send_cmd(0x21);
	//send_cmd(0x00);
	//send_cmd(0x7f);
	//send_cmd(0x22);
	//send_cmd(0x00);
	//send_cmd(0x07);

	for(int i=0;i<8;i++) {
		send_cmd(0xb0 + i);
		send_cmd(0x02);
		send_cmd(0x10);
		for(int j=0;j<128;j++) {
			send_data(buffer[i*128 + j]);
		}
	}
}

void reset() {
	icosoc_oled_rst(1);
        for (int i = 0; i < 20000; i++) asm volatile ("");
	icosoc_oled_rst(0);
        for (int i = 0; i < 200000; i++) asm volatile ("");
	icosoc_oled_rst(1);
}

void init() {
	printf("Initialising\n");
	icosoc_oled_cs(1);
	icosoc_oled_rst(1);
	icosoc_oled_prescale(4); // 5Mhz
	icosoc_oled_mode(false, false);
	printf("Reset\n");
	reset();
        printf("Reset pin is %ld\n", icosoc_oled_getrst());
	send_cmd(0xAE); // Display off
	send_cmd(0xA6); // Normal display
	send_cmd(0x02); // Set low column 2
	send_cmd(0x40); // Set start line zero
	send_cmd(0xC8); // Com scan Dec
	send_cmd(0x81); // Set contrast
	send_cmd(0xCF); // Set contrast
	//send_cmd(0x9F); // Set contrast
	send_cmd(0xA4); // Set Display all on resume
	send_cmd(0xA8); // Set multiplex
	send_cmd(0x3F); // Set multiplex
	send_cmd(0xD3); // Set Display offset
	send_cmd(0x00); // Set Display offset
	send_cmd(0xD5); // Set Display Clock Div
	send_cmd(0x80); // Set Display Clock Div
	send_cmd(0xD9); // Set precharge
	send_cmd(0xF1); // Set precharge
	//send_cmd(0x22); // Set precharge
	send_cmd(0xDA); // Set Comp Ins
	send_cmd(0x12); // Set Comp Ins
	send_cmd(0xDB); // Set Vcom Detect
	send_cmd(0x40); // Set Vcom Detect
	send_cmd(0x8D); // Charge pump
	send_cmd(0x14); // Charge pump
	//send_cmd(0x10); // Charge pump
	send_cmd(0x20); // Memory mode
	send_cmd(0x00); // Memory mode
	send_cmd(0xA1); // Seg remap 1
	send_cmd(0x2E); // Deactivate scroll.
	send_cmd(0xAF); // Switch on

	printf("Initialisation done\n");

}

int main()
{
	init();
	drawText(20,30,"Hello World!",1);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		display();	

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

