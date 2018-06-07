#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

static int console_getc()
{
        while (1) { 
                int c = *(volatile uint32_t*)0x30000000;
                if (c >= 0) return c;
        }
}

int main()
{
	int row = 1, col = 0;
	char *text = "Send characters to /dev/ttyusb0";
		
	icosoc_vga_setcolor(0xfff);
	for(int i=0;text[i];i++) icosoc_vga_setchar(col++,row,text[i]);
	row = 2; col = 0;

	for(;;) {
		int c = console_getc();
		if (c == '\n') {
			col = 0;
			if (++row == 30) row = 0;
		} else if (c == '\b') {
			if (--col < 0) {
				if (row != 0) {
					row--;
					col = 79;
				} else col = 0;
			}
			icosoc_vga_setchar(col,row,' ');
		} else if (c == 0) {
			int r = console_getc() & 0xf;
			int g = console_getc() & 0xf;
			int b = console_getc() & 0xf;
			icosoc_vga_setcolor((r << 8) + (g << 4) + b);
		} else {
			icosoc_vga_setchar(col++, row, c);
			if (col == 80) {
				col = 0;
				if (++row == 30) row = 0;
			}
		}	
	}	
}

