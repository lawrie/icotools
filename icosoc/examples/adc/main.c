#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);

		uint8_t data = icosoc_adc1_get();

		printf("Data is %02x\n", data);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

