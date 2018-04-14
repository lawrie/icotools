CC = $(RISCV_TOOLS_PREFIX)gcc
CFLAGS = --std=gnu99 -Os -MD -Wall
OBJS = main.o icosoc.o

default: help

appimage.elf: $(OBJS)
	$(CC) -T $(LDSCRIPT) -o appimage.elf $(OBJS)
	chmod -x appimage.elf

appimage.hex: appimage.elf
	$(RISCV_TOOLS_PREFIX)objcopy -O verilog appimage.elf appimage.hex
	chmod -x appimage.hex

icosoc.mk: icosoc.cfg
	../../icosoc.py

clean::
	rm -f appimage.hex appimage.elf $(OBJS) *.d

include icosoc.mk
-include *.d

