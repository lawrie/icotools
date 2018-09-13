#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"

#define ADDRESS 0x52

uint8_t read() {
	uint32_t status;
	icosoc_i2c_read_no_wr(ADDRESS);
	do {
		status = icosoc_i2c_status();
		//printf("Status is %lx\n", status);
	} while((status & 0x80000000) != 0);
	return status & 0xFF;
}

void send_cmd(uint8_t r, uint8_t d) {
	uint32_t status;

	icosoc_i2c_write1(ADDRESS, r, d);
	do {
		status = icosoc_i2c_status();
		//printf("Send status is %lx\n", status);
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

		printf("Received %02x %02x %02x %02x %02x %02x\n", jx, jy, ax, ay, az, rest & 3);
		//printf("Received %02x\n", jx);

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}

