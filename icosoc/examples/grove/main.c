#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"

int main()
{
	icosoc_button1_dir(0); 
	icosoc_button2_dir(0); 
	icosoc_buzzer_dir(1); 
	icosoc_led_dir(1); 
	icosoc_servo_setmaxcnt(ICOSOC_CLOCK_FREQ_HZ / 50);
	icosoc_servo_setcounter(0);
	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(icosoc_button1_get() << 1 | icosoc_button2_get());
		icosoc_buzzer_set(icosoc_button2_get() || icosoc_button1_get());
		icosoc_led_set(icosoc_button2_get() || icosoc_button1_get());
		printf("Counter %ld\n", icosoc_servo_getcounter());
		printf("Max count %ld\n", icosoc_servo_getmaxcnt());
		if (icosoc_button2_get()) {
			uint32_t count = icosoc_servo_getcounter();
			icosoc_servo_setcounter(count+10000);
		}
		for (int i = 0; i < 100000; i++)
			asm volatile ("");

	}
}

