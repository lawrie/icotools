#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

#define offset 50 
#define divisor 7

int main()
{

	icosoc_servo_setmaxcnt(ICOSOC_CLOCK_FREQ_HZ / 50);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		uint16_t duty = ((i+offset) * 100 / divisor / 256);
		uint32_t pos = (icosoc_servo_getmaxcnt() / divisor / 256) * (i + offset);

		icosoc_servo_setoffcnt(pos);
		printf("Duty: %d\n", duty);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

