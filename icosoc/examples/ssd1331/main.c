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
	send_cmd(0xA0); // Set remap
	send_cmd(0x72); // RGB
	send_cmd(0xA1); // Startline
	send_cmd(0x00); // 0
	send_cmd(0xA2); // Display offer
	send_cmd(0x00); // 0
	send_cmd(0xA4); // Normal display
	send_cmd(0xA8); // Set multiplex
	send_cmd(0x3F); // 1/64 duty
	send_cmd(0xAD); // Set master
	send_cmd(0x8E); // 
	send_cmd(0xB0); // Set power mode
	send_cmd(0x0B); // 
	send_cmd(0xB1); // Precharge
	send_cmd(0x31); // 
	send_cmd(0xB3); // Set clock div
	send_cmd(0xF0); // 
	send_cmd(0x8A); // Precharge A
	send_cmd(0x64); // 
	send_cmd(0x8B); // Precharge B
	send_cmd(0x78); // 
	send_cmd(0x8B); // Precharge level
	send_cmd(0x3A); // 
	send_cmd(0xBE); // Vcomh
	send_cmd(0x3E); // 
	send_cmd(0x87); // Master current
	send_cmd(0x06); // 
	send_cmd(0x81); // Set Contrast A
	send_cmd(0x91); // 
	send_cmd(0x82); // Set Contrast B
	send_cmd(0x50); // 
	send_cmd(0x83); // Set Contrast C
	send_cmd(0x7D); // 
	send_cmd(0xAF); // Switch on

	printf("Initialisation done\n");

}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT)) return;
	
	send_cmd(0x15);
	send_cmd(x);
	send_cmd(WIDTH-1);

	send_cmd(0x75);
	send_cmd(y);
	send_cmd(HEIGHT-1);

	send_data(color >> 8);
	send_data(color);
}

int main()
{
	init();
	fillRect(0,0,WIDTH,HEIGHT,0);


	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);

		drawText(20,30,"Hello World!",(i << 8) + i);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

