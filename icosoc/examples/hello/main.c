#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	icosoc_ledstrip_dir(0xffff);

	// generate a 38 kHz signal with pwm0
	icosoc_pwm0_setmaxcnt(ICOSOC_CLOCK_FREQ_HZ / 38000);

	*(volatile uint32_t*)(0x10000000) = 0x12345678;
	unsigned int hram_value = *(volatile uint32_t*)(0x10000000);
	printf("HyperRAM readback: %08x\n", hram_value);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		printf("[%02x] Hello World!\n", i);

		char buffer[100];
		int buffer_len;

		buffer_len = snprintf(buffer, 100, "[%02x] Hello World!\r\n", i);
		icosoc_ser0_write(buffer, buffer_len);

		icosoc_ledstrip_set(1 << (i % 16));

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

		if (i % 2) {
			// enable pwm0 (50:50 duty cycle)
			icosoc_pwm0_setoffcnt(icosoc_pwm0_getmaxcnt() / 2);
		} else {
			// disable pwm0
			icosoc_pwm0_setoffcnt(0);
		}
	}
}

