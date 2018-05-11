#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"

void send_rgb_cmd(uint8_t r, uint8_t d) {
	uint32_t status;

	icosoc_i2c_write(0x62, r, d, 0);
	do {
		status = icosoc_i2c_status();
	} while ((status >> 31) != 0);
	printf("Status is %lx\n",icosoc_i2c_status());
}

void setRGB(uint8_t r, uint8_t g, uint8_t b) {
	send_rgb_cmd(0,0);
	send_rgb_cmd(1,0);
	send_rgb_cmd(0x08,0xaa);
	send_rgb_cmd(4,r);
	send_rgb_cmd(3,g);
	send_rgb_cmd(2,b);
}

void send_cmd(uint8_t d) {
	uint32_t status;

	icosoc_i2c_write(0x3e, 0x80, d, 0);
	do {
		status = icosoc_i2c_status();
	} while ((status >> 31) != 0);
	printf("Status is %lx\n",icosoc_i2c_status());
}

void send_data(uint8_t d) {
        uint32_t status;

        icosoc_i2c_write(0x3c, 0x40, d, 0);
        do {
                status = icosoc_i2c_status();
        } while ((status >> 31) != 0);
}

void setText(const char *text) {
	int count = 0, row = 0;

	send_cmd(0x01);
	for (int i = 0; i < 100000; i++) asm volatile ("");
	send_cmd(0x08 | 0x04);
	send_cmd(0x28);
	for (int i = 0; i < 100000; i++) asm volatile ("");
	for(int i=0;text[i];i++) {
		uint8_t c = text[i];
		if (c == '\n' || count == 16) {
			count = 0;
			row++;
			if (row == 2) break;
			send_cmd(0xc0);
			if (c == '\n') continue;
		}
		count++;
		send_data(c);
	}
}
	
int main()
{
	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		setRGB(0,128,64);		
		setText("Hello World!");
		
		for (int i = 0; i < 100000; i++)
			asm volatile ("");
	}
}

