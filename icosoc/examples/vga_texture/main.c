#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

#define ADDRESS 0x52

int sx, sy, x_inc = 10, y_inc = 10;

uint8_t read() {
	uint32_t status;
	icosoc_i2c_read_no_wr(ADDRESS);
	do {
		status = icosoc_i2c_status();
		printf("Status is %lx\n", status);
	} while((status & 0x80000000) != 0);
	return status & 0xFF;
}

void send_cmd(uint8_t r, uint8_t d) {
	uint32_t status;

	icosoc_i2c_write1(ADDRESS, r, d);
	do {
		status = icosoc_i2c_status();
		printf("Send status is %lx\n", status);
	} while ((status >> 31) != 0);
}

int main()
{

	send_cmd(0x40, 0);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		
		send_cmd(0x00, 0x00);	        
		uint8_t jx = read();
		uint8_t jy = read();
		uint8_t ax = read();
		uint8_t ay = read();
		uint8_t az = read();
		uint8_t rest = read();

		int8_t y_speed = (123 - jy) / 8;
		int8_t x_speed = (jx - 127) / 8;

		printf("Received %02x %02x %02x %02x %02x %02x\n", jx, jy, ax, ay, az, rest & 3);

		icosoc_vga_set_sprite_pos(sx, sy);
		sy = (sy + y_speed) & 0x1ff;
		sx = (sx + x_speed) & 0x1ff;;

		for (int j = 0; j < 1000000; j++)
			asm volatile ("");

	}
}

