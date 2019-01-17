// #### This file is auto-generated from icosoc.py. Do not edit! ####


#ifndef ICOSOC_H
#define ICOSOC_H

#include <stdint.h>
#include <stdbool.h>

#define ICOSOC_CLOCK_FREQ_HZ 20000000

static inline void icosoc_irq(void(*irq_handler)(uint32_t,uint32_t*)) {
    *((uint32_t*)8) = (uint32_t)irq_handler;
}

extern uint32_t icosoc_maskirq(uint32_t mask);
extern uint32_t icosoc_timer(uint32_t ticks);

static inline void icosoc_sbreak() {
    asm volatile ("sbreak" : : : "memory");
}

static inline void icosoc_leds(uint8_t value)
{
    *(volatile uint32_t *)0x20000000 = value;
}



static inline void icosoc_pwm0_setcounter(uint32_t val)
{
    *(volatile uint32_t*)(0x20000000 + 3 * 0x10000) = val;
}

static inline void icosoc_pwm0_setmaxcnt(uint32_t val)
{
    *(volatile uint32_t*)(0x20000004 + 3 * 0x10000) = val;
}

static inline void icosoc_pwm0_setoncnt(uint32_t val)
{
    *(volatile uint32_t*)(0x20000008 + 3 * 0x10000) = val;
}

static inline void icosoc_pwm0_setoffcnt(uint32_t val)
{
    *(volatile uint32_t*)(0x2000000c + 3 * 0x10000) = val;
}

static inline uint32_t icosoc_pwm0_getcounter()
{
    return *(volatile uint32_t*)(0x20000000 + 3 * 0x10000);
}

static inline uint32_t icosoc_pwm0_getmaxcnt()
{
    return *(volatile uint32_t*)(0x20000004 + 3 * 0x10000);
}

static inline uint32_t icosoc_pwm0_getoncnt()
{
    return *(volatile uint32_t*)(0x20000008 + 3 * 0x10000);
}

static inline uint32_t icosoc_pwm0_getoffcnt()
{
    return *(volatile uint32_t*)(0x2000000c + 3 * 0x10000);
}

void icosoc_ser0_read(void *data, int len);
void icosoc_ser0_write(const void *data, int len);
int icosoc_ser0_read_nb(void *data, int maxlen);
int icosoc_ser0_write_nb(const void *data, int maxlen);

static inline void icosoc_ledstrip_set(uint32_t bitmask) {
    *(volatile uint32_t*)(0x20000000 + 2 * 0x10000) = bitmask;
}

static inline uint32_t icosoc_ledstrip_get() {
    return *(volatile uint32_t*)(0x20000000 + 2 * 0x10000);
}

static inline void icosoc_ledstrip_dir(uint32_t bitmask) {
    *(volatile uint32_t*)(0x20000004 + 2 * 0x10000) = bitmask;
}


#endif /* ICOSOC_H */

