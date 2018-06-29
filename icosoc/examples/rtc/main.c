#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"

#define ADDRESS 0x68

char *days[] = {"Unknown", "Monday", "Tuesday", "Wednesday", 
              "Thursday", "Friday", "Saturday", "Sunday"};

uint8_t read_reg(uint8_t r) {
	uint32_t status;
	icosoc_i2c_read(ADDRESS, r);
	do {
		status = icosoc_i2c_status();
	} while((status & 0x80000000) != 0);
	return status & 0xFF;
}

void send_cmd(uint8_t r, uint8_t d) {
	uint32_t status;

	icosoc_i2c_write1(ADDRESS, r, d);
	do {
		status = icosoc_i2c_status();
	} while ((status >> 31) != 0);
}

int main()
{
	send_cmd(0x00, 0); // Set seconds to zero
	send_cmd(0x01, 0x12); // Set minutes to zero
	send_cmd(0x02, 0x18); // Set hour to 6pm
	send_cmd(0x03, 0x05); // Friday
	send_cmd(0x04, 0x29); // 29th
	send_cmd(0x05, 0x06); // June
	send_cmd(0x06, 0x18); // 2018

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		
		uint8_t s = read_reg(0x00);
		uint8_t m = read_reg(0x01);
		uint8_t h = read_reg(0x02);
		uint8_t w = read_reg(0x03);
		uint8_t d = read_reg(0x04);
		uint8_t mo = read_reg(0x05);
		uint8_t y = read_reg(0x06);

		if (w > 7) w = 0;

		printf("Date is %s %02x/%02x/20%02x %02x:%02x:%02x\n", days[w],d,mo,y,h,m,s);

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}

