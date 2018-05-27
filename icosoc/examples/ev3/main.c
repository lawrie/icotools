#include <stdio.h>
#include <stdint.h>
#include "icosoc.h"
#include <string.h>

uint8_t get_byte() {
	uint8_t buffer[1];
	int16_t n;
	do {
		n = icosoc_ev3_read_nb(buffer,1);
	} while (n == 0);
	return buffer[0];
}

uint8_t get_data_byte() {
	uint8_t buffer[1];
	uint16_t n;

	do {
		n = icosoc_data_read_nb(buffer,1);
	} while (n == 0);
	//printf("Read %d bytes\n",n);
	return buffer[0];
}

int main()
{
	int8_t status = 0;
	uint8_t cmd;
	uint8_t type; 
	uint8_t modes; 
	uint8_t views; 
	uint8_t checksum;
	uint8_t mode = 0;
	uint32_t speed;
	uint8_t nack[] = {0x02};
	uint8_t ack[] = {0x04};

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);
		if (status <2) cmd = get_byte();		
		else {
			//printf("Reading data byte\n");
			cmd = get_data_byte();
			if (cmd != 0) printf("Read data %x\n", cmd);
		}

		if (status == 0 && cmd == 0x40) {
			printf("Received CMD_TYPE\n");
			checksum = 0xff ^ cmd;
			type = get_byte();
			printf("Type is %x\n", type);
			checksum ^= type;
			uint8_t cs = get_byte();
			printf("Checksum is %x\n", cs);
			if (checksum == cs) {
				printf("Checksum match\n");
				status = 1;
			} else
				printf("Checksum failed\n");

		} else if (status == 1 && cmd == 0x49) {
			checksum = 0xff ^ cmd;
			modes = get_byte();
		 	checksum ^= modes;
			views = get_byte();
			checksum ^= views;	
			printf("Modes %d, views %d\n",modes,views);
			uint8_t cs = get_byte();
			if (checksum == cs) {
				modes++;
				printf("Modes checksum match\n");
			} else
				printf("Modes checksum failed\n");

		} else if (status == 1 && cmd == 0x52) {
			checksum = 0xff ^ cmd;
			uint8_t speed1 = get_byte();
		 	checksum ^= speed1;
			uint8_t speed2 = get_byte();
			checksum ^= speed2;	
			uint8_t speed3 = get_byte();
		 	checksum ^= speed3;
			uint8_t speed4 = get_byte();
			checksum ^= speed4;	
			speed = (speed4 << 24) + (speed3 << 16) + (speed2 << 8) + speed4;
			printf("Speed %ld\n",speed);
			uint8_t cs = get_byte();
			if (checksum == cs) {
				printf("Speed checksum match\n");
			} else
				printf("Speed checksum failed\n");
		} else if (status == 1 && (cmd & 0xc0) == 0x80) {
			checksum = 0xff ^ cmd;
			uint8_t lll = (cmd & 0x38) >> 3;
			uint8_t l = 1 << lll;
			uint8_t mode = cmd & 0x07;
			uint8_t typ = get_byte();
			checksum ^= typ;
			printf("Info type %d, l %d mode %d\n", typ, l, mode);
			for(int i=0;i<l;i++) {
				uint8_t info = get_byte();
				checksum ^= info;
			}
			uint8_t cs = get_byte();
			if (checksum == cs) {
				printf("Info checksum match\n");
			} else
				printf("Info checksum failed\n");
		} else if (status == 1 && cmd == 0x04) { //ACK
			printf("ACK received\n");
			icosoc_ev3_write(ack,1); // Send an ACK back
			for (int i = 0; i < 200000; i++)
				asm volatile ("");
			status = 2;
		} else if (status == 2 && (cmd & 0xc0) == 0xc0) {
			checksum = 0xff ^ cmd;
			uint8_t lll = (cmd & 0x38) >> 3;
			uint8_t l = 1 << lll;
			uint8_t mode = cmd & 0x07;
			printf("Data received, mode %d, l %d\n",mode,l);
			for(int i=0;i<l;i++) {
				uint8_t dat = get_data_byte();
				checksum ^= dat;
			}
			uint8_t cs = get_data_byte();
			if (checksum == cs) {
				printf("Data checksum match\n");
			} else
				printf("Data checksum failed\n");
		}
	}
}

