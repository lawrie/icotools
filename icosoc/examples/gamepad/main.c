#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	icosoc_pad_dir(0); // All input, with pullup

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(icosoc_pad_get());

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

