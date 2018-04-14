#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		int32_t count = icosoc_rotary0_get();
		printf("Value : %ld\n", count);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

