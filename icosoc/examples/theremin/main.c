#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		icosoc_ping0_trigger();

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

		int32_t distance = icosoc_ping0_get();
		printf("Distance : %ld\n", distance);

		icosoc_tone0_setperiod(1000000/(distance * 20));
	}
}

