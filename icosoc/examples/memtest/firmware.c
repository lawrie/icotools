#include <stdint.h>
#include <stdbool.h>

#define MAX_MEMSIZE_KB 256

static inline void setled(int v)
{
	*(volatile uint32_t*)0x20000000 = v;
}

static void console_putc(int c)
{
	*(volatile uint32_t*)0x30000000 = c;
}

static void console_puts(const char *s)
{
	while (*s)
		*(volatile uint32_t*)0x30000000 = *(s++);
}

static void console_putd(int v)
{
	if (v < 0) {
		console_putc('-');
		v *= -1;
	}

	bool printed_digits = false;
	for (int i = 1000000000; i > 0; i /= 10)
		if (printed_digits || v >= i) {
			printed_digits = true;
			console_putc('0' + v / i);
			v = v % i;
		}
	
	if (!printed_digits)
		console_putc('0');
}

uint32_t x32;
static uint32_t xorshift32() {
	x32 ^= x32 << 13;
	x32 ^= x32 >> 17;
	x32 ^= x32 << 5;
	return x32;
}

int main()
{
	bool found_error = false;

	uint32_t ledstrip = ~0;
	*(volatile uint32_t*)(0x20000004 + 1 * 0x10000) = ~0;
	*(volatile uint32_t*)(0x20000000 + 1 * 0x10000) = ledstrip;

	setled(1);
	console_puts("[memtest] detecting memory size.. ");

	for (int i = MAX_MEMSIZE_KB-1; i >= 0; i--)
		*(volatile uint32_t*)((64 + i) * 1024) = i;
	
	int memsize_kb;
	for (memsize_kb = 0; memsize_kb < MAX_MEMSIZE_KB; memsize_kb++)
		if (*(volatile uint32_t*)((64 + memsize_kb) * 1024) != memsize_kb)
			break;

	console_putd(memsize_kb);
	console_puts(" kB\n");

	setled(2);
	console_puts("[memtest] testing.. ");

	for (int k = 0; k < 10; k++)
	{
		x32 = 1234567890 + k;
		for (int i = 0; i < memsize_kb * 1024; i += 4)
			*(volatile uint32_t*)(64 * 1024 + i) = xorshift32();

		x32 = 1234567890 + k;
		for (int i = 0; i < memsize_kb * 1024; i += 4)
			if (*(volatile uint32_t*)(64 * 1024 + i) != xorshift32())
				found_error = true;
	}

	console_puts(found_error ? "ERROR\n" : "OK\n");
	console_putc(0);

	for (int k = 0;; k = (k+1) & 7)
	{
		for (int i = 0; i < 500000; i++)
			asm volatile ("");

		setled((k & 1) ? (found_error ? 4 : 3) : 0);

		if (k < 4)
			ledstrip = ~ledstrip;

		*(volatile uint32_t*)(0x20000000 + 1 * 0x10000) = ledstrip;
	}
}

