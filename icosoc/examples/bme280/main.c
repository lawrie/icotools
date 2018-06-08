#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"

static uint16_t T1, P1;
static int16_t T2, T3, P2, P3, P4, P5, P6, P7, P8, P9;
static int32_t t_fine;

uint8_t read_reg(uint8_t r) {
	uint32_t status;
	icosoc_i2c_read(0x77, r);
	do {
		status = icosoc_i2c_status();
	} while((status & 0x80000000) != 0);
	return status & 0xFF;
}

uint32_t read24(uint8_t r) {
	uint8_t b0 = read_reg(r);
	uint8_t b1 = read_reg(r+1);
	uint8_t b2 = read_reg(r+2);

	return (b0 << 16) + (b1 << 8) + b2;
}

uint16_t read16(uint8_t r) {
	uint8_t b0 = read_reg(r);
	uint8_t b1 = read_reg(r+1);

	return (b0 << 8) + b1;
}

uint16_t read16_LE(uint8_t r) {
	uint8_t b0 = read_reg(r);
	uint8_t b1 = read_reg(r+1);

	return (b1 << 8) + b0;
}

void send_cmd(uint8_t r, uint8_t d) {
	uint32_t status;

	icosoc_i2c_write1(0x77, r, d);
	do {
		status = icosoc_i2c_status();
	} while ((status >> 31) != 0);
}

float readTemperature(void) {
	int32_t adc_T = read24(0xFA) >> 4;

 	int32_t t1 = ((((adc_T>>3) - ((int32_t) T1 <<1))) * 
			((int32_t) T2)) >> 11;

	int32_t t2 = (((((adc_T>>4) - ((int32_t) T1)) * ((adc_T>>4) - 
			((int32_t) T1))) >> 12) * ((int32_t) T3)) >> 14;

	t_fine = t1 + t2;

	float T = (t_fine * 5 + 128) >> 8;

	return T/100;
}

float readPressure(void) {

	readTemperature();

	int32_t adc_P = read24(0xF7) >> 4;

	int64_t t1 = ((int64_t)t_fine) - 128000;
	int64_t t2 = (t1 * t1 * (int64_t) P6) + ((t1 * (int64_t) P5) << 17) + (((int64_t) P4) << 35);

	t1 = ((t1 * t1 * (int64_t) P3)>>8) + ((t1 * (int64_t) P2) << 12);
	t1 = (((((int64_t) 1) << 47) + t1)) * ((int64_t) P1) >> 33;

	if (t1 == 0) return 0;

	int64_t p = 1048576 - adc_P;

	p = (((p << 31) - t2) * 3125) /t1;

	t1 = (((int64_t) P9) * (p >>13) * (p >> 13)) >> 25;
	t2 = (((int64_t) P8) * p) >> 19;

	p = ((p + t1 + t2) >> 8) + (((int64_t) P7) << 4);

	return (float) p/256;
}

int main()
{
	printf("Chip id is %x\n",read_reg(0xD0));

	send_cmd(0xF5,0x00); // Filter off, 0.5ms standby
	send_cmd(0xF4,0xB7); // Register control, mode normal, x16

	T1 = read16_LE(0x88);
	T2 = read16_LE(0x8A);
	T3 = read16_LE(0x8C);

	P1 = read16_LE(0x8E);
	P2 = read16_LE(0x90);
	P3 = read16_LE(0x92);
	P4 = read16_LE(0x94);
	P5 = read16_LE(0x96);
	P6 = read16_LE(0x98);
	P7 = read16_LE(0x9A);
	P8 = read16_LE(0x9C);
	P9 = read16_LE(0x9E);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);

		float temperature = readTemperature();
			
		printf("Temperature is %2.2f\n",temperature);

		float pressure = readPressure();
			
		printf("Pressure is %4.2f\n",pressure/100);

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}

