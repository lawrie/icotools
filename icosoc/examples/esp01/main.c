#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"
#include <string.h>

static int console_getc()
{
	while (1) {
		int c = *(volatile uint32_t*)0x30000000;
		if (c >= 0) return c;
	}
}

int main()
{
	char buffer[80];
	int16_t msg_idx;
	char message[80];

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		
		msg_idx = 0;
		for(;;) {
			int16_t c = console_getc();
			message[msg_idx++] = c;
			if (msg_idx == 79 || c == '\n') break;
		}
		message[msg_idx] = 0;

		printf("Sending %s",message);
		icosoc_esp_write(message,strlen(message));

		for (int i = 0; i < 1000000; i++)
			asm volatile ("");

		int16_t n = icosoc_esp_read_nb(buffer,79);
		buffer[n] = 0;
		printf("Received %d bytes: \n%s\n",n, buffer);
		
		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}

