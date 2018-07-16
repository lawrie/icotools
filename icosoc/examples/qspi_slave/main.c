#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		// Set the byte to send back
		icosoc_qspi_transmit(0xbd);
		// Call receive twice to see if we lose bytes
		uint32_t r = icosoc_qspi_receive();
		r = icosoc_qspi_receive();
		uint32_t count = r >> 8;
		uint8_t data = r & 0xff;

		printf("Received %02x %ld\n", data, count);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

