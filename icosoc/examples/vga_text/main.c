#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	char *text = "Hello World!";
	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);

		for (int i=0;text[i] > 0; i++) {
			icosoc_vga_setchar(20+i,10,text[i]);
			printf("%c", text[i]);
		}
		printf("\n");

		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

