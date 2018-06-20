#include <stdio.h>
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
	//printf("Initialising\n");
	icosoc_oled_cs(1);
	icosoc_oled_rst(1);
	icosoc_oled_prescale(4); // 5Mhz
	icosoc_oled_mode(false, false);
	//printf("Reset\n");
	reset();
        //printf("Reset pin is %ld\n", icosoc_oled_getrst());
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

	//printf("Initialisation done\n");

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

int main()
{
	char cmd[2];
	uint8_t dir1 = 1, dir2 = 1;
	uint32_t saved_power;
	bool stopped = false;
	uint16_t label_color = 0xf81f;
	uint16_t text_color = 0xffe0;
	cmd[0] = ' ';
	cmd[1] = 0;

	init();
	fillRect(0,0,WIDTH,HEIGHT,0);

	// Generate a 1.5 khz signal
	icosoc_power1_setmaxcnt(ICOSOC_CLOCK_FREQ_HZ / 1500);
	icosoc_power2_setmaxcnt(ICOSOC_CLOCK_FREQ_HZ / 1500);

	uint32_t  power = icosoc_power1_getmaxcnt() * 3 / 4;

	// Set motor directions
	icosoc_dir_dir(0x03); // Direction pins output, switches input
		
	drawText(0,0,"Power:", label_color);
	drawText(0,10,"Last cmd:", label_color);
	drawText(0,20,"Count1:",label_color);
	drawText(0,30,"Count2:",label_color);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
	

		uint32_t n = icosoc_ble_read_nb(cmd,1);
		if (n > 0) {
			//printf("Command is %02x\n", cmd[0]);
			char c = cmd[0];
			stopped = false;

			switch(c) {
				case 'f': {
					//printf("Forwards\n");
					dir1 = 1;
					dir2 = 1;
					break;
				}
				
				case 'b': {
					//printf("Backwards\n");
					dir1 = 0;
					dir2 = 0;
					break;
				}

				case 'l': {
					//printf("Left\n");
					dir1 = 0;
					dir2 = 1;
					break;
				}

				case 'r': {
					//printf("Right\n");
					dir1 = 1;
					dir2 = 0;
					break;
				}

				case 's': {
					//printf("Stopped\n");
					stopped = true;
					break;
				}
			}
		}
	
		icosoc_dir_set((dir1 << 1) | dir2); // Set pins to switches

		icosoc_power1_setoffcnt(stopped ? 0 : power);
		icosoc_power2_setoffcnt(stopped ? 0 : power);

		int32_t count1 = icosoc_rot1_get();
		int32_t count2 = icosoc_rot2_get();
		
		char buff[11];
		sprintf(buff,"%5ld",power);
		drawText(64,0,buff,text_color);
		drawText(64,10,cmd,text_color);
		sprintf(buff,"%7ld",count1);
		drawText(64,20,buff,text_color);
		sprintf(buff,"%7ld",count2);
		drawText(64,30,buff,text_color);
		
		//printf("Motor1 : %ld, motor2 : %ld, power %ld\n", count1, count2, power);

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");

	}
}

