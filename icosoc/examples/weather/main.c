#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"
#include "gfx.h"

void send_cmd(uint8_t r) {
	icosoc_oled_cs(1);
	icosoc_oled_dc(0);
	icosoc_oled_cs(0);

	uint8_t d = icosoc_oled_xfer(r);
	icosoc_oled_cs(1);
	//printf("Command %x\n", d);
}

void send_data(uint8_t d) {
	icosoc_oled_cs(1);
	icosoc_oled_dc(1);
	icosoc_oled_cs(0);

	uint8_t r2 = icosoc_oled_xfer(d);

	icosoc_oled_cs(1);
	//printf("Data %x\n", r);
}

void reset() {
	icosoc_oled_rst(1);
        for (int i = 0; i < 20000; i++) asm volatile ("");
	icosoc_oled_rst(0);
        for (int i = 0; i < 200000; i++) asm volatile ("");
	icosoc_oled_rst(1);
}

void init() {
	printf("Initialising\n");
	icosoc_oled_cs(1);
	icosoc_oled_rst(1);
	icosoc_oled_prescale(4); // 5Mhz
	icosoc_oled_mode(false, false);
	printf("Reset\n");
	reset();
        printf("Reset pin is %ld\n", icosoc_oled_getrst());
	send_cmd(0xFD); // Command lock
	send_data(0x12); // 
	send_cmd(0xFD); // Command lock
	send_data(0xB1); // 
	send_cmd(0xAE); // Display off
	send_cmd(0xB3); // Set clock div
	send_cmd(0xF1); // 
	send_cmd(0xCA); // Mux ratio
	send_data(0x7F); // 
	send_cmd(0xA0); // Set remap
	send_data(0x74); // RGB
	send_cmd(0x15);
	send_data(0x00);
	send_data(0x7F);
	send_cmd(0x75);
	send_data(0x00);
	send_data(0x7F);
	send_cmd(0xA1); // Startline
	send_data(0x00); // 0
	send_cmd(0xA2); // Display offset
	send_data(0x00); // 0
	send_cmd(0xB5); // Set GPIO
	send_data(0x00); // 0
	send_cmd(0xAB); // Funcion select
	send_data(0x01); // internal diode drop
	send_cmd(0xB1); // Precharge
	send_cmd(0x32); // 
	send_cmd(0xBE); // Vcomh
	send_cmd(0x05); // 
	send_cmd(0xA6); // Normal display
	send_cmd(0xC1); // Set Contrast ABC
	send_data(0xC8); // 
	send_data(0x80); // 
	send_data(0xC8); // 
	send_cmd(0xC7); // Set Contrast Master
	send_data(0x0F); // 
	send_cmd(0xB4); // Set VSL
	send_data(0xA0); // 
	send_data(0xB5); // 
	send_data(0x55); // 
	send_cmd(0x86); // Precharge 2
	send_data(0x01); // 
	send_cmd(0xAF); // Switch on

	printf("Initialisation done\n");

}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT)) return;
	
	send_cmd(0x15);
	send_data(x);
	send_data(WIDTH-1);

	send_cmd(0x75);
	send_data(y);
	send_data(HEIGHT-1);

	send_cmd(0x5C);

	send_data(color >> 8);
	send_data(color);
}

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

void write_reg(uint8_t r, uint8_t d) {
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
	char msg[20];

	init();
	fillRect(0,0,WIDTH,HEIGHT,0);

	printf("Chip id is %x\n",read_reg(0xD0));

	write_reg(0xF5,0x00); // Filter off, 0.5ms standby
	write_reg(0xF4,0xB7); // Register control, mode normal, x16

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
		sprintf(msg,"Temperature:");
		drawText(0,10,msg,0xFFE0);
		sprintf(msg,"%2.2f",temperature);
		drawText(80,10,msg,0xFFFF);

		float pressure = readPressure();
			
		printf("Pressure is %4.2f",pressure/100);
		sprintf(msg,"Pressure:");
		drawText(0,20,msg,0xFFE0);
		sprintf(msg,"%4.2f",pressure/100);
		drawText(80,20,msg,0xFFFF);

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}

