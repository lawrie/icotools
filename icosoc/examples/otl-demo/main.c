#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	char buffer[100];
	int buffer_len;

	for (uint8_t i = 0;; i++)
	{
		icosoc_ssd0_set(i);
		printf("[%02x] Hello World!\n", i);

		for (int i = 0; i < 100000; i++)
			asm volatile ("");
	}
}

