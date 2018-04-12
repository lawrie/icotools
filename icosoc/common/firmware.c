#include <stdint.h>
#include <stdbool.h>

static inline void setled(int v)
{
	*(volatile uint32_t*)0x20000000 = v;
}

static void console_putc(int c)
{
	*(volatile uint32_t*)0x30000000 = c;
}

static void console_puth32(uint32_t v)
{
	for (int i = 0; i < 8; i++) {
		int d = v >> 28;
		console_putc(d < 10 ? '0' + d : 'a' + d - 10);
		v = v << 4;
	}
}

static void console_puth8(uint8_t v) __attribute__((unused));

static void console_puth8(uint8_t v)
{
	v &= 0xff;
	int d = v >> 4;
	v &= 0x0f;

	console_putc(d < 10 ? '0' + d : 'a' + d - 10);
	console_putc(v < 10 ? '0' + v : 'a' + v - 10);
}

static void console_puts(const char *s)
{
	while (*s)
		*(volatile uint32_t*)0x30000000 = *(s++);
}

static int console_getc_timeout()
{
	while (1) {
		int c = *(volatile uint32_t*)0x30000000;
		if (c >= 0) return c;
	}
}

static int console_getc()
{
	while (1) {
		int c = *(volatile uint32_t*)0x30000000;
		if (c >= 0) return c;
	}
}

static bool ishex(char ch)
{
	if ('0' <= ch && ch <= '9') return true;
	if ('a' <= ch && ch <= 'f') return true;
	if ('A' <= ch && ch <= 'F') return true;
	return false;
}

static int hex2int(char ch)
{
	if ('0' <= ch && ch <= '9') return ch - '0';
	if ('a' <= ch && ch <= 'f') return ch - 'a' + 10;
	if ('A' <= ch && ch <= 'F') return ch - 'A' + 10;
	return -1;
}

int main()
{
	// detect verilog testbench
	if (((*(volatile uint32_t*)0x20000000) & 0x80000000) != 0) {
		console_puts("Bootloader> ");
		console_puts("TESTBENCH\n");
		return 0;
	}

	console_puts("Bootloader> ");
	uint8_t *memcursor = (uint8_t*)(64 * 1024);
	int bytecount = 0;

	while (1)
	{
		char ch = console_getc_timeout();

		if (ch == 0 || ch == '@')
		{
			/*if (bytecount) {
				console_puts("\nWritten 0x");
				console_puth32(bytecount);
				console_puts(" bytes at 0x");
				console_puth32((uint32_t)memcursor);
				console_puts(".\nBootloader> ");
			}*/

			if (ch == 0) {
				console_puts("RUN\n");
				break;
			}

			int newaddr = 0;
			while (1) {
				ch = console_getc();
				if (!ishex(ch)) break;
				newaddr = (newaddr << 4) | hex2int(ch);
			}

			memcursor = (uint8_t*)newaddr;
                        //console_puts("\nNew address: ");
			//console_puth32((uint32_t)memcursor);
                        //console_puts("\n");
			bytecount = 0;
			continue;
		}

		if (ishex(ch))
		{
			char ch2 = console_getc();

			if (ishex(ch2)) {
				if (bytecount % 1024 == 0)
					console_putc('.');
				memcursor[bytecount++] = (hex2int(ch) << 4) | hex2int(ch2);
				continue;
			}

			console_putc(ch);
			ch = ch2;
			goto prompt;
		}

		if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
			continue;

	prompt:
		console_putc(ch);
		console_putc('\n');
		console_puts("Bootloader> ");
	}

	return 0;
}

