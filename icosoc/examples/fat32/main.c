#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "icosoc.h"

// ========================================================================

bool sdcard_ccs_mode;

static void sdcard_cs(bool enable)
{
	if (enable) {
		// printf("SD Card EN=1\n");
		icosoc_sdcard_cs(0);
	} else {
		// printf("SD Card EN=0\n");
		icosoc_sdcard_cs(1);
	}
}

static uint8_t sdcard_xfer(uint8_t value)
{
	uint8_t ret = icosoc_sdcard_xfer(value);
	// printf("SD Card Xfer: %02x %02x\n", value, ret);
	return ret;
}

static const uint8_t sdcard_crc7_table[256] = {
	0x00, 0x12, 0x24, 0x36, 0x48, 0x5a, 0x6c, 0x7e,
	0x90, 0x82, 0xb4, 0xa6, 0xd8, 0xca, 0xfc, 0xee,
	0x32, 0x20, 0x16, 0x04, 0x7a, 0x68, 0x5e, 0x4c,
	0xa2, 0xb0, 0x86, 0x94, 0xea, 0xf8, 0xce, 0xdc,
	0x64, 0x76, 0x40, 0x52, 0x2c, 0x3e, 0x08, 0x1a,
	0xf4, 0xe6, 0xd0, 0xc2, 0xbc, 0xae, 0x98, 0x8a,
	0x56, 0x44, 0x72, 0x60, 0x1e, 0x0c, 0x3a, 0x28,
	0xc6, 0xd4, 0xe2, 0xf0, 0x8e, 0x9c, 0xaa, 0xb8,
	0xc8, 0xda, 0xec, 0xfe, 0x80, 0x92, 0xa4, 0xb6,
	0x58, 0x4a, 0x7c, 0x6e, 0x10, 0x02, 0x34, 0x26,
	0xfa, 0xe8, 0xde, 0xcc, 0xb2, 0xa0, 0x96, 0x84,
	0x6a, 0x78, 0x4e, 0x5c, 0x22, 0x30, 0x06, 0x14,
	0xac, 0xbe, 0x88, 0x9a, 0xe4, 0xf6, 0xc0, 0xd2,
	0x3c, 0x2e, 0x18, 0x0a, 0x74, 0x66, 0x50, 0x42,
	0x9e, 0x8c, 0xba, 0xa8, 0xd6, 0xc4, 0xf2, 0xe0,
	0x0e, 0x1c, 0x2a, 0x38, 0x46, 0x54, 0x62, 0x70,
	0x82, 0x90, 0xa6, 0xb4, 0xca, 0xd8, 0xee, 0xfc,
	0x12, 0x00, 0x36, 0x24, 0x5a, 0x48, 0x7e, 0x6c,
	0xb0, 0xa2, 0x94, 0x86, 0xf8, 0xea, 0xdc, 0xce,
	0x20, 0x32, 0x04, 0x16, 0x68, 0x7a, 0x4c, 0x5e,
	0xe6, 0xf4, 0xc2, 0xd0, 0xae, 0xbc, 0x8a, 0x98,
	0x76, 0x64, 0x52, 0x40, 0x3e, 0x2c, 0x1a, 0x08,
	0xd4, 0xc6, 0xf0, 0xe2, 0x9c, 0x8e, 0xb8, 0xaa,
	0x44, 0x56, 0x60, 0x72, 0x0c, 0x1e, 0x28, 0x3a,
	0x4a, 0x58, 0x6e, 0x7c, 0x02, 0x10, 0x26, 0x34,
	0xda, 0xc8, 0xfe, 0xec, 0x92, 0x80, 0xb6, 0xa4,
	0x78, 0x6a, 0x5c, 0x4e, 0x30, 0x22, 0x14, 0x06,
	0xe8, 0xfa, 0xcc, 0xde, 0xa0, 0xb2, 0x84, 0x96,
	0x2e, 0x3c, 0x0a, 0x18, 0x66, 0x74, 0x42, 0x50,
	0xbe, 0xac, 0x9a, 0x88, 0xf6, 0xe4, 0xd2, 0xc0,
	0x1c, 0x0e, 0x38, 0x2a, 0x54, 0x46, 0x70, 0x62,
	0x8c, 0x9e, 0xa8, 0xba, 0xc4, 0xd6, 0xe0, 0xf2
};

static uint8_t sdcard_crc7(uint8_t crc, uint8_t data)
{
	return sdcard_crc7_table[crc ^ data];
}

static uint16_t sdcard_crc16(uint16_t crc, uint8_t data)
{
	uint16_t x = (crc >> 8) ^ data;
	x ^= x >> 4;
	return (crc << 8) ^ (x << 12) ^ (x << 5) ^ x;
}

static uint8_t sdcard_cmd_r1(uint8_t cmd, uint32_t arg)
{
	uint8_t r1;

	sdcard_cs(true);

	sdcard_xfer(0x40 | cmd);
	sdcard_xfer(arg >> 24);
	sdcard_xfer(arg >> 16);
	sdcard_xfer(arg >> 8);
	sdcard_xfer(arg);

	uint8_t crc = 0;
	crc = sdcard_crc7(crc, 0x40 | cmd);
	crc = sdcard_crc7(crc, arg >> 24);
	crc = sdcard_crc7(crc, arg >> 16);
	crc = sdcard_crc7(crc, arg >> 8);
	crc = sdcard_crc7(crc, arg);
	sdcard_xfer(crc | 1);

	do {
		r1 = sdcard_xfer(0xff);
	} while (r1 == 0xff);

	sdcard_cs(false);
	return r1;
}

static uint8_t sdcard_cmd_rw(uint8_t cmd, uint32_t arg)
{
	uint8_t r1;

	sdcard_cs(true);

	sdcard_xfer(0x40 | cmd);
	sdcard_xfer(arg >> 24);
	sdcard_xfer(arg >> 16);
	sdcard_xfer(arg >> 8);
	sdcard_xfer(arg);

	uint8_t crc = 0;
	crc = sdcard_crc7(crc, 0x40 | cmd);
	crc = sdcard_crc7(crc, arg >> 24);
	crc = sdcard_crc7(crc, arg >> 16);
	crc = sdcard_crc7(crc, arg >> 8);
	crc = sdcard_crc7(crc, arg);
	sdcard_xfer(crc | 1);

	do {
		r1 = sdcard_xfer(0xff);
	} while (r1 == 0xff);

	return r1;
}

static uint8_t sdcard_cmd_r37(uint8_t cmd, uint32_t arg, uint32_t *r37)
{
	uint8_t r1;

	sdcard_cs(true);

	sdcard_xfer(0x40 | cmd);
	sdcard_xfer(arg >> 24);
	sdcard_xfer(arg >> 16);
	sdcard_xfer(arg >> 8);
	sdcard_xfer(arg);

	uint8_t crc = 0;
	crc = sdcard_crc7(crc, 0x40 | cmd);
	crc = sdcard_crc7(crc, arg >> 24);
	crc = sdcard_crc7(crc, arg >> 16);
	crc = sdcard_crc7(crc, arg >> 8);
	crc = sdcard_crc7(crc, arg);
	sdcard_xfer(crc | 1);

	do {
		r1 = sdcard_xfer(0xff);
	} while (r1 == 0xff);

	for (int i = 0; i < 4; i++)
		*r37 = (*r37 << 8) | sdcard_xfer(0xff);

	sdcard_cs(false);
	return r1;
}

static void sdcard_init()
{
	uint8_t r1;
	uint32_t r37;

	sdcard_cs(false);
	icosoc_sdcard_prescale(20);
	icosoc_sdcard_mode(true, true);

	for (int i = 0; i < 10; i++)
		sdcard_xfer(0xff);

	r1 = sdcard_cmd_r1(0, 0);

	if (r1 != 0x01) {
		printf("Unexpected SD Card CMD0 R1: %02x\n", r1);
		while (1) { }
	}

	r1 = sdcard_cmd_r1(59, 1);

	if (r1 != 0x01) {
		printf("Unexpected SD Card CMD59 R1: %02x\n", r1);
		while (1) { }
	}

	r1 = sdcard_cmd_r37(8, 0x1ab, &r37);
	if (r1 != 0x01 || (r37 & 0xfff) != 0x1ab) {
		printf("Unexpected SD Card CMD8 R1 / R7: %02x %08x\n", r1, (int)r37);
		while (1) { }
	}

	r1 = sdcard_cmd_r37(58, 0, &r37);

	if (r1 != 0x01) {
		printf("Unexpected SD Card CMD58 R1: %02x\n", r1);
		while (1) { }
	}

	if ((r37 & 0x00300000) == 0) {
		printf("SD Card doesn't support 3.3V! OCR reg: %08x\n", (int)r37);
		while (1) { }
	}

	for (int i = 0;; i++)
	{
		// ACMD41, set HCS
		sdcard_cmd_r1(55, 0);
		r1 = sdcard_cmd_r1(41, 1 << 30);

		if (r1 == 0x00)
			break;

		if (r1 != 0x01) {
			printf("Unexpected SD Card ACMD41 R1: %02x\n", r1);
			while (1) { }
		}

		if (i == 10000) {
			printf("Timeout on SD Card ACMD41.\n");
			while (1) { }
		}
	}

	r1 = sdcard_cmd_r37(58, 0, &r37);

	if (r1 != 0x00) {
		printf("Unexpected SD Card CMD58 R1: %02x\n", r1);
		while (1) { }
	}

	sdcard_ccs_mode = !!(r37 & (1 << 30));

	r1 = sdcard_cmd_r1(16, 512);

	if (r1 != 0x00) {
		printf("Unexpected SD Card CMD16 R1: %02x\n", r1);
		while (1) { }
	}
}

static void sdcard_read(uint8_t *data, uint32_t blockaddr)
{
	if (!sdcard_ccs_mode)
		blockaddr <<= 9;

	uint8_t r1 = sdcard_cmd_rw(17, blockaddr);

	if (r1 != 0x00) {
		printf("Unexpected SD Card CMD17 R1: %02x\n", r1);
		while (1) { }
	}

	while (1) {
		r1 = sdcard_xfer(0xff);
		if (r1 == 0xfe) break;
		if (r1 == 0xff) continue;
		printf("Unexpected SD Card CMD17 data token: %02x\n", r1);
		while (1) { }
	}

	uint16_t crc = 0x0;
	for (int i = 0; i < 512; i++) {
		data[i] = sdcard_xfer(0xff);
		crc = sdcard_crc16(crc, data[i]);
	}

	crc = sdcard_crc16(crc, sdcard_xfer(0xff));
	crc = sdcard_crc16(crc, sdcard_xfer(0xff));

	if (crc != 0) {
		printf("CRC Error while reading from SD Card!\n");
		while (1) { }
	}

	sdcard_cs(false);
}

static void sdcard_write(const uint8_t *data, uint32_t blockaddr)
{
	if (!sdcard_ccs_mode)
		blockaddr <<= 9;

	uint8_t r1 = sdcard_cmd_rw(24, blockaddr);

	if (r1 != 0x00) {
		printf("Unexpected SD Card CMD24 R1: %02x\n", r1);
		while (1) { }
	}

	sdcard_xfer(0xff);
	sdcard_xfer(0xfe);

	uint16_t crc = 0x0;
	for (int i = 0; i < 512; i++) {
		crc = sdcard_crc16(crc, data[i]);
		sdcard_xfer(data[i]);
	}

	sdcard_xfer(crc >> 8);
	sdcard_xfer(crc);

	r1 = sdcard_xfer(0xff);
	if ((r1 & 0x0f) != 0x05) {
		printf("Unexpected SD Card CMD24 data response: %02x\n", r1);
		while (1) { }
	}

	while (sdcard_xfer(0xff) != 0xff) {
		/* waiting for sd card */
	}

	sdcard_cs(false);
}

// ========================================================================

int main()
{
	sdcard_init();
	printf("SD Card Initialized.\n\n");

	printf("Master Boot Record:\n\n");

	uint8_t buffer[512];

	sdcard_read(buffer, 0);

	for (int i = 0; i < 512; i++)
		printf("%02x%c", buffer[i], i % 32 == 31 ? '\n' : ' ');
	printf("\n");

        if (buffer[510] == 0x55 && buffer[511] == 0xAA) 
		printf("MBR is valid.\n\n");

        uint8_t *part = &buffer[446];

	printf("Boot flag: %d\n", part[0]);
	printf("Type code: 0x%x\n", part[4]);

        uint32_t *lba_begin = (uint32_t *) &part[8];
	uint32_t Partition_LBA_Begin = lba_begin[0];

	printf("LBA begin: %ld\n", Partition_LBA_Begin);
	printf("Number of sectors: %ld\n", lba_begin[1]);

	printf("\nVolume ID:\n\n");
	
	sdcard_read(buffer, Partition_LBA_Begin);

	for (int i = 0; i < 512; i++)
		printf("%02x%c", buffer[i], i % 32 == 31 ? '\n' : ' ');
	printf("\n");

        if (buffer[510] == 0x55 && buffer[511] == 0xAA) 
		printf("Volume ID is valid\n\n");

	uint16_t Number_of_Reserved_Sectors = *((uint16_t *) &buffer[0x0e]);
	printf("Number of reserved sectors: %d\n", Number_of_Reserved_Sectors);

	uint8_t Number_of_FATs = buffer[0x10];
	uint32_t Sectors_Per_FAT = *((uint32_t *) &buffer[0x24]);
	uint8_t sectors_per_cluster = buffer[0x0d];
	uint32_t root_dir_first_cluster = *((uint32_t *) &buffer[0x2c]);

	printf("Number of FATs: %d\n", Number_of_FATs);
	printf("Sectors per FAT: %ld\n", Sectors_Per_FAT);
	printf("Sectors per cluster: %d\n", sectors_per_cluster);
	printf("Root dir first cluster: %ld\n", root_dir_first_cluster);
 
	uint32_t fat_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors;
	printf("fat begin lba: %ld\n", fat_begin_lba);

	uint32_t cluster_begin_lba = Partition_LBA_Begin + Number_of_Reserved_Sectors + (Number_of_FATs * Sectors_Per_FAT);
	printf("cluster begin lba: %ld\n", cluster_begin_lba);

	uint32_t root_dir_lba = cluster_begin_lba + (root_dir_first_cluster - 2) * sectors_per_cluster;
	printf("root dir lba: %ld\n", root_dir_lba);

	sdcard_read(buffer, root_dir_lba);

	printf("\nRoot directory first sector:\n\n");

	for (int i = 0; i < 512; i++)
		printf("%02x%c", buffer[i], i % 32 == 31 ? '\n' : ' ');
	printf("\n");

	uint8_t filename[13], first_file[13];
	filename[8] = '.';
	filename[12] = 0;

	printf("Root directory:\n\n");
	uint16_t first_cluster_lo, first_cluster_hi;
	uint32_t first_cluster, file_size, first_file_cluster = 0;
	uint8_t attrib;

	for(int i=0; buffer[i];i+=32) {
		if (buffer[i] != 0xe5) {
			if (buffer[i+11] != 0x0f) {
				strncpy((char *) filename,(const char *) &buffer[i],8);
				strncpy((char *) &filename[9], (const char *) &buffer[i+8],3);
				attrib = buffer[i+0x0b];
				first_cluster_hi = *((uint16_t *) &buffer[i+0x14]);
				first_cluster_lo = *((uint16_t *) &buffer[i+0x1a]);
				first_cluster = (first_cluster_hi << 16) + first_cluster_lo;
				file_size = *((uint32_t *) &buffer[i+0x1c]);
				if ((attrib & 0x1f) == 0) {
					first_file_cluster = first_cluster;
					strcpy((char *) first_file, (const char *) filename);
				}
				printf("%s %x %ld %ld\n", filename, attrib, first_cluster, file_size);
			}
		}
	}

	printf("\nFirst file, first cluster: %ld\n", first_file_cluster);

	// Read first sector of the FAT
	uint32_t fat[128];

	sdcard_read((uint8_t *) fat, fat_begin_lba);

	printf("\nFAT:\n\n");

	for (int i = 0; i < 128; i++)
		printf("%08lx%c", fat[i], i % 8 == 7 ? '\n' : ' ');
	printf("\n");

	printf("\nReading file %s\n\n", first_file);

	uint32_t first_lba = cluster_begin_lba + (first_file_cluster - 2) * sectors_per_cluster;
	sdcard_read(buffer, first_lba);

	printf("\nFile first sector %ld:\n\n", first_lba);

	for (int i = 0; i < 512; i++)
		printf("%02x%c", buffer[i], i % 32 == 31 ? '\n' : ' ');
	printf("\n");

	for(uint32_t i =first_file_cluster;i < 128;i = fat[i]) {
		uint32_t lba = cluster_begin_lba + (i-2) * sectors_per_cluster;
		printf("Cluster: %ld, lba: %ld\n", i, lba);
		for(uint32_t j = 0; j<sectors_per_cluster;j++) {
			sdcard_read(buffer, lba+j);
		}
	}

	printf("\nDONE.\n");
	return 0;
}

