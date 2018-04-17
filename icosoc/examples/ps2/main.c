#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{

	for (uint8_t i = 0;; i++)
	{
		int32_t code = icosoc_ps2_get();
		if (code > 0) printf("Scan code : %lx\n", code);
	}
}

