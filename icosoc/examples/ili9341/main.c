#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"
#include "gfx.h"

void send_cmd(uint8_t r) {
	icosoc_ili9341_cs(1);
	icosoc_ili9341_dc(0);
	icosoc_ili9341_cs(0);

	uint8_t d = icosoc_ili9341_xfer(r);
	icosoc_ili9341_cs(1);
	//printf("\n%02x", d);
}

void send_data(uint8_t d) {
	icosoc_ili9341_cs(1);
	icosoc_ili9341_dc(1);
	icosoc_ili9341_cs(0);

	uint8_t r2 = icosoc_ili9341_xfer(d);

	icosoc_ili9341_cs(1);
	//if (d != 0) printf(" %02x", d);
}

void reset() {
	icosoc_ili9341_rst(1);
        for (int i = 0; i < 20000; i++) asm volatile ("");
	icosoc_ili9341_rst(0);
        for (int i = 0; i < 200000; i++) asm volatile ("");
	icosoc_ili9341_rst(1);
}

void init() {
	printf("Initialising\n");
	icosoc_ili9341_cs(1);
	icosoc_ili9341_rst(1);
	icosoc_ili9341_prescale(4); // 5Mhz
	icosoc_ili9341_mode(false, false);
	printf("Reset\n");
	reset();
        printf("Reset pin is %ld\n", icosoc_ili9341_getrst());

        send_cmd(0x01);

        send_cmd(0x12);

        send_cmd(0x28);

	send_cmd(0xEF); 
	send_data(0x03); 
	send_data(0x80); 
	send_data(0x02); 

	send_cmd(0xCF); 
	send_data(0x00); 
	send_data(0xC1); 
	send_data(0x30); 

	send_cmd(0xED);
	send_data(0x64); 
	send_data(0x03); 
	send_data(0x12); 
	send_data(0x81); 

	send_cmd(0xE8); 
	send_data(0x85); 
	send_data(0x00); 
	send_data(0x78); 

	send_cmd(0xCB); 
	send_data(0x39); 
	send_data(0x2C); 
	send_data(0x00); 
	send_data(0x34); 
	send_data(0x02); 

	send_cmd(0xF7); 
	send_data(0x20);  

	send_cmd(0xEA); 
	send_data(0x00); 
	send_data(0x00); 

	send_cmd(0xC0);
	send_data(0x23);

	send_cmd(0xC1);
	send_data(0x10);

	send_cmd(0xC5); 
	send_data(0x3e); 
	send_data(0x28); 

	send_cmd(0xC6); 
	send_data(0x86); 

	send_cmd(0x36); 
	send_data(0x28);

	send_cmd(0x37); 
	send_data(0x00); 

	send_cmd(0x3A); 
	send_data(0x55); 

	send_cmd(0xB1); 
	send_data(0x00); 
	send_data(0x18);
 
	send_cmd(0xB6); 
	send_data(0x08); 
	send_data(0x82); 
	send_data(0x27); 

	send_cmd(0xF2); 
	send_data(0x00); 

	send_cmd(0x26); 
	send_data(0x01); 

	send_cmd(0xE0); 
	send_data(0x0F); 
	send_data(0x31); 
	send_data(0x2B); 
	send_data(0x0C); 
	send_data(0x0E); 
	send_data(0x08); 
	send_data(0x4E); 
	send_data(0xF1); 
	send_data(0x37); 
	send_data(0x07); 
	send_data(0x10); 
	send_data(0x03); 
	send_data(0x0E); 
	send_data(0x09); 
	send_data(0x00); 

	send_cmd(0xE1); 
	send_data(0x00);  
	send_data(0x0E);  
	send_data(0x14);  
	send_data(0x03);  
	send_data(0x11);  
	send_data(0x07);  
	send_data(0x31);  
	send_data(0xC1);  
	send_data(0x48);  
	send_data(0x08);  
	send_data(0x0F);  
	send_data(0x0C);  
	send_data(0x31);  
	send_data(0x36);  
	send_data(0x0F);  

        send_cmd(0x13);

        send_cmd(0xF2);
        send_data(0x00);

	send_cmd(0x11); 
 
	send_cmd(0x29); 

	printf("\nInitialisation done\n");
}

void clear(uint16_t c) {
        send_cmd(0x2A);
        send_data(0x00);
        send_data(0x00);
        send_data(0x01);
        send_data(0x3F);

        send_cmd(0x2B);
        send_data(0x00);
        send_data(0x00);
        send_data(0x00);
        send_data(0xEF);

        send_cmd(0x2C);

        // Not sure why we have to send more than a screen's worth of data
        for(int i=0; i< HEIGHT+10; i++) {
        	for (int j = 0; j<WIDTH; j++) {
        		send_data(c >> 8);
        		send_data(c);
		}
	}
}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT)) return;
	
	send_cmd(0x2A);
        send_data(x >> 8);
	send_data(x);
	send_data(x >> 8);
	send_data(x);

	send_cmd(0x2B);
        send_data(y >> 8);
	send_data(y);
	send_data(y >> 8);
	send_data(y);

	send_cmd(0x2C);

	send_data(color >> 8);
	send_data(color);
}

int main()
{
	init();
        clear(0x00);
        printf("\n");

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);

		drawText(20,30,"Hello World!",(i << 8) + i);

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");

	}
}

