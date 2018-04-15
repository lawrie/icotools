#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		uint32_t tone = i << 3;
		uint32_t period = 1000000 / tone;

		icosoc_tone0_setperiod(period);
		printf("Tone: %ld\n", tone);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

