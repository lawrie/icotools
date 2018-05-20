#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "icosoc.h"

// ========================================================================

static void ad1_cs(bool enable)
{
	if (enable) {
		icosoc_ad1_cs(0);
	} else {
		icosoc_ad1_cs(1);
	}
}

static uint8_t ad1_xfer1()
{
	uint8_t ret = icosoc_ad1_xfer1();

	return ret;
}

static uint8_t ad1_xfer0()
{
	uint8_t ret = icosoc_ad1_xfer0();

	return ret;
}

static uint16_t read_ad0() {
	ad1_cs(true);

	uint8_t msb = ad1_xfer0();
	uint8_t lsb = ad1_xfer0();

	ad1_cs(false);

	return (msb << 8) + lsb;
}

static uint16_t read_ad1() {
	ad1_cs(true);

	uint8_t msb = ad1_xfer1();
	uint8_t lsb = ad1_xfer1();

	ad1_cs(false);

	return (msb << 8) + lsb;
}

// ========================================================================

int main()
{

	icosoc_ad1_prescale(20);
	icosoc_ad1_mode(false, false);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
	
		printf("D0 = %d, D1 = %d\n", read_ad0() * 3263 / 4095,
					     read_ad1() * 3263 / 4095);
	
		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}
