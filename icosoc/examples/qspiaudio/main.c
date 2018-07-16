#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	uint32_t lost = 0, rcvd = 0;

	for (uint16_t i = 0;; i++)
	{
		icosoc_leds(i);
		// Set the byte to send back
		icosoc_qspi_transmit(0x00);
		// Call receive 
		uint32_t r = icosoc_qspi_receive();
		uint32_t count = r >> 8;
		uint8_t data = r & 0xff;

		if (count == 1) {
			rcvd++;
			icosoc_audio_play(data);
		} else if (count != 0) lost++;

		if (i == 0) printf("Received: %ld, lost: %ld\n",rcvd,lost);

		//for (int i = 0; i < 100000; i++)
		//	asm volatile ("");

	}
}

