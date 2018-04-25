#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

void send_cmd(uint8_t r, uint8_t d) {
	uint32_t status;

	icosoc_i2c_write(0x3c, 0x00, r, d);
	do {
		status = icosoc_i2c_status();
	} while ((status >> 31) != 0);
	printf("Status is %lx\n",icosoc_i2c_status());
}

void send_data(uint8_t d1, uint8_t d2) {
        uint32_t status;

        icosoc_i2c_write(0x3c, 0x40, d1, d2);
        do {
                status = icosoc_i2c_status();
        } while ((status >> 31) != 0);
        printf("Status is %lx\n",icosoc_i2c_status());
}

int main()
{
	printf("Initialising\n");

	send_cmd(0xAE, 0x00); // Display off
	send_cmd(0x00, 0x00); // Set low column
	send_cmd(0x40, 0x00); // Set start line
	send_cmd(0x81, 0xCF); // Set contrast
	send_cmd(0xA4, 0x00); // Set Display all on resum
	send_cmd(0xA8, 0x3F); // Set multiples
	send_cmd(0xD3, 0x00); // Set Display offset
	send_cmd(0xD5, 0x80); // Set Display Clock Div
	send_cmd(0xD9, 0xF1); // Set precharge
	send_cmd(0xDA, 0x12); // Set Comp Ins
	send_cmd(0xDB, 0x40); // Set Vcom Detect
	send_cmd(0x8D, 0x14); // Charge pump
	send_cmd(0x20, 0x00); // Memory mode
	send_cmd(0xAF, 0x00); // Switch on

	printf("Initialisation done\n");

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);

		send_data(0xAA, 0x55);
		
		printf("Status is %lx\n",icosoc_i2c_status());

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

