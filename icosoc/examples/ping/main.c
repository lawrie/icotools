#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		icosoc_ping0_trigger();

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");

		int32_t count = icosoc_ping0_get();
		printf("Value : %ld\n", count);
	}
}

