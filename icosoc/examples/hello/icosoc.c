// #### This file is auto-generated from icosoc.py. Do not edit! ####


#include "icosoc.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

asm (
".global icosoc_maskirq\n"
"icosoc_maskirq:\n"
".word 0x0605650b\n" // picorv32_maskirq_insn(a0, a0)
"ret\n"
);

asm (
".global icosoc_timer\n"
"icosoc_timer:\n"
".word 0x0a05650b\n" // picorv32_timer_insn(a0, a0)
"ret\n"
);



void icosoc_ser0_read(void *data, int len)
{
    while (len > 0) {
        int n = icosoc_ser0_read_nb(data, len);
        data += n, len -= n;
    }
}

void icosoc_ser0_write(const void *data, int len)
{
    while (len > 0) {
        int n = icosoc_ser0_write_nb(data, len);
        data += n, len -= n;
    }
}

int icosoc_ser0_read_nb(void *data, int maxlen)
{
    uint8_t *p = data;
    int len = *(volatile uint32_t*)(0x20000004 + 1 * 0x10000);
    if (len > maxlen) len = maxlen;

    for (int i = 0; i < len; i++)
        p[i] = *(volatile uint32_t*)(0x20000000 + 1 * 0x10000);

    return len;
}

int icosoc_ser0_write_nb(const void *data, int maxlen)
{
    const uint8_t *p = data;
    int len = *(volatile uint32_t*)(0x20000008 + 1 * 0x10000);
    if (len > maxlen) len = maxlen;

    for (int i = 0; i < len; i++)
        *(volatile uint32_t*)(0x20000000 + 1 * 0x10000) = p[i];

    return len;
}

// An extremely minimalist syscalls.c for newlib
// Based on riscv newlib libgloss/riscv/machine/syscall.h
// Written by Clifford Wolf.

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define UNIMPL_FUNC(_f) ".globl " #_f "\n.type " #_f ", @function\n" #_f ":\n"

asm (
	".text\n"
	".align 2\n"
	UNIMPL_FUNC(_open)
	UNIMPL_FUNC(_openat)
	UNIMPL_FUNC(_lseek)
	UNIMPL_FUNC(_stat)
	UNIMPL_FUNC(_lstat)
	UNIMPL_FUNC(_fstatat)
	UNIMPL_FUNC(_isatty)
	UNIMPL_FUNC(_access)
	UNIMPL_FUNC(_faccessat)
	UNIMPL_FUNC(_link)
	UNIMPL_FUNC(_unlink)
	UNIMPL_FUNC(_execve)
	UNIMPL_FUNC(_getpid)
	UNIMPL_FUNC(_fork)
	UNIMPL_FUNC(_kill)
	UNIMPL_FUNC(_wait)
	UNIMPL_FUNC(_times)
	UNIMPL_FUNC(_gettimeofday)
	UNIMPL_FUNC(_ftime)
	UNIMPL_FUNC(_utime)
	UNIMPL_FUNC(_chown)
	UNIMPL_FUNC(_chmod)
	UNIMPL_FUNC(_chdir)
	UNIMPL_FUNC(_getcwd)
	UNIMPL_FUNC(_sysconf)
	"j unimplemented_syscall\n"
);

void unimplemented_syscall()
{
	const char *p = "Unimplemented system call called!\n";
	while (*p)
		*(volatile int*)0x10000000 = *(p++);
	asm volatile ("sbreak");
	__builtin_unreachable();
}

ssize_t _read(int file, void *ptr, size_t len)
{
	// All reads read from console
	unsigned char *p = ptr;
	for (int i = 0; i < len;) {
		int v = *(volatile int*)0x30000000;
		if (v >= 0)
			p[i++] = v;
		else if (i)
			return i;
	}
	return len;
}

ssize_t _write(int file, const void *ptr, size_t len)
{
	// All writes go to console
	const unsigned char *p = ptr;
	for (int i = 0; i < len; i++)
		*(volatile int*)0x30000000 = p[i];
	return len;
}

int _close(int file)
{
	// close is called before _exit()
	return 0;
}

int _fstat(int file, struct stat *st)
{
	// fstat is called during libc startup
	errno = ENOENT;
	return -1;
}

void *_sbrk(ptrdiff_t incr)
{
	extern unsigned char _end[];	// Defined by linker
	static unsigned long heap_end;

	if (heap_end == 0)
		heap_end = (long)_end;

	heap_end += incr;
	return (void *)(heap_end - incr);
}

void _exit(int exit_status)
{
	asm volatile ("sbreak");
	__builtin_unreachable();
}

