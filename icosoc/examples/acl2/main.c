#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "icosoc.h"

// ========================================================================

static void acl_cs(bool enable)
{
	if (enable) {
		icosoc_acl_cs(0);
	} else {
		icosoc_acl_cs(1);
	}
}

static uint8_t acl_xfer(uint8_t value)
{
	uint8_t ret = icosoc_acl_xfer(value);

	return ret;
}

static void write_reg(uint8_t r, uint8_t v) 
{
	acl_cs(true);

	acl_xfer(0x0A);
	acl_xfer(r);
	acl_xfer(v);

	acl_cs(false);
}

static uint8_t read_reg(uint8_t r) 
{
	acl_cs(true);

	acl_xfer(0x0B);
	uint8_t res = acl_xfer(r);

	acl_cs(false);

	return res;
}

static int16_t read_long(uint8_t r) 
{
	acl_cs(true);

	acl_xfer(0x0B);
	acl_xfer(r);
	uint8_t lsb = acl_xfer(0);
	uint8_t msb = acl_xfer(0);

	acl_cs(false);

	return (msb << 8) + lsb;
}

static int16_t get_x()
{
	return read_long(0x0E);
}

static int16_t get_y()
{
	return read_long(0x10);
}

static int16_t get_z()
{
	return read_long(0x12);
}

// ========================================================================

int main()
{

	icosoc_acl_prescale(4); // 5Mhz, I think
	icosoc_acl_mode(false, false); // SPI mode 0
	acl_cs(false);

	write_reg(0x2d,0x02); // Set measurement mode

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
	
		printf("X = %d, Y = %d, Z = %d\n", get_x(), get_y(), get_z());
	
		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}
