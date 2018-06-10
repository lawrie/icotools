#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "icosoc.h"
#include "gfx.h"

#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)
#define decodeVcselPeriod(reg_val)      (((reg_val) + 1) << 1)
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)

enum vcselPeriodType { VcselPeriodPreRange, VcselPeriodFinalRange };

void send_cmd(uint8_t r) {
	icosoc_oled_cs(1);
	icosoc_oled_dc(0);
	icosoc_oled_cs(0);

	icosoc_oled_xfer(r);
	icosoc_oled_cs(1);
}

void send_data(uint8_t d) {
	icosoc_oled_cs(1);
	icosoc_oled_dc(1);
	icosoc_oled_cs(0);

	icosoc_oled_xfer(d);

	icosoc_oled_cs(1);
}

void reset() {
	icosoc_oled_rst(1);
        for (int i = 0; i < 20000; i++) asm volatile ("");
	icosoc_oled_rst(0);
        for (int i = 0; i < 200000; i++) asm volatile ("");
	icosoc_oled_rst(1);
}

void init() {
	printf("Initialising\n");
	icosoc_oled_cs(1);
	icosoc_oled_rst(1);
	icosoc_oled_prescale(4); // 5Mhz
	icosoc_oled_mode(false, false);
	printf("Reset\n");
	reset();
        printf("Reset pin is %ld\n", icosoc_oled_getrst());
	send_cmd(0xFD); // Command lock
	send_data(0x12); // 
	send_cmd(0xFD); // Command lock
	send_data(0xB1); // 
	send_cmd(0xAE); // Display off
	send_cmd(0xB3); // Set clock div
	send_cmd(0xF1); // 
	send_cmd(0xCA); // Mux ratio
	send_data(0x7F); // 
	send_cmd(0xA0); // Set remap
	send_data(0x74); // RGB
	send_cmd(0x15);
	send_data(0x00);
	send_data(0x7F);
	send_cmd(0x75);
	send_data(0x00);
	send_data(0x7F);
	send_cmd(0xA1); // Startline
	send_data(0x00); // 0
	send_cmd(0xA2); // Display offset
	send_data(0x00); // 0
	send_cmd(0xB5); // Set GPIO
	send_data(0x00); // 0
	send_cmd(0xAB); // Funcion select
	send_data(0x01); // internal diode drop
	send_cmd(0xB1); // Precharge
	send_cmd(0x32); // 
	send_cmd(0xBE); // Vcomh
	send_cmd(0x05); // 
	send_cmd(0xA6); // Normal display
	send_cmd(0xC1); // Set Contrast ABC
	send_data(0xC8); // 
	send_data(0x80); // 
	send_data(0xC8); // 
	send_cmd(0xC7); // Set Contrast Master
	send_data(0x0F); // 
	send_cmd(0xB4); // Set VSL
	send_data(0xA0); // 
	send_data(0xB5); // 
	send_data(0x55); // 
	send_cmd(0x86); // Precharge 2
	send_data(0x01); // 
	send_cmd(0xAF); // Switch on

	printf("Initialisation done\n");

}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
        if((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT)) return;
	
	send_cmd(0x15);
	send_data(x);
	send_data(WIDTH-1);

	send_cmd(0x75);
	send_data(y);
	send_data(HEIGHT-1);

	send_cmd(0x5C);

	send_data(color >> 8);
	send_data(color);
}

uint8_t readReg(uint8_t r) {
	uint32_t status;
	icosoc_i2c_read(0x29, r);
	do {
		status = icosoc_i2c_status();
	} while((status & 0x80000000) != 0);
	printf("Read status is %lx\n", icosoc_i2c_status());
	return status & 0xFF;
}

uint32_t read24(uint8_t r) {
	uint8_t b0 = readReg(r);
	uint8_t b1 = readReg(r+1);
	uint8_t b2 = readReg(r+2);

	return (b0 << 16) + (b1 << 8) + b2;
}

uint16_t readReg16Bit(uint8_t r) {
	uint8_t b0 = readReg(r);
	uint8_t b1 = readReg(r+1);

	return (b0 << 8) + b1;
}

uint16_t read16_LE(uint8_t r) {
	uint8_t b0 = readReg(r);
	uint8_t b1 = readReg(r+1);

	return (b1 << 8) + b0;
}

void writeReg(uint8_t r, uint8_t d) {
	uint32_t status;

	icosoc_i2c_write1(0x29, r, d);
	do {
		status = icosoc_i2c_status();
	} while ((status >> 31) != 0);
	printf("Write status is %lx\n", icosoc_i2c_status());
}

void writeReg16Bit(uint8_t reg, uint16_t value)
{
  writeReg(reg,(value >> 8) & 0xFF); // value high byte
  writeReg(reg+1, value & 0xFF); // value low byte
}

void writeReg32Bit(uint8_t reg, uint16_t value)
{
  writeReg(reg,(value >> 24) & 0xFF); // value high byte
  writeReg(reg,(value >> 16) & 0xFF); // value high byte
  writeReg(reg,(value >> 8) & 0xFF); // value high byte
  writeReg(reg+1, value & 0xFF); // value low byte
}

void writeMulti(uint8_t reg, uint8_t const *src, uint8_t count) {
	for(int i=0;i<count;i++) writeReg(reg+i,src[i]);
}

static uint8_t stop_variable;
uint32_t measurement_timing_budget_us;

void startContinuous(uint32_t period_ms)
{
  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);
  writeReg(0x91, stop_variable);
  writeReg(0x00, 0x01);
  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);

  if (period_ms != 0)
  {
    // continuous timed mode

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() begin

    uint16_t osc_calibrate_val = readReg16Bit(0xF8);

    if (osc_calibrate_val != 0)
    {
      period_ms *= osc_calibrate_val;
    }

    writeReg32Bit(0x04, period_ms);

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() end

    writeReg(0x00, 0x04); // VL53L0X_REG_SYSRANGE_MODE_TIMED
  }
  else
  {
    // continuous back-to-back mode
    writeReg(0x00, 0x02); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
  }
}

uint16_t readRangeContinuousMillimeters(void)
{
  //startTimeout();
  while ((readReg(0x13) & 0x07) == 0)
  {
    /*if (checkTimeoutExpired())
    {
      did_timeout = true;
      return 65535;
    }*/
  }

  // assumptions: Linearity Corrective Gain is 1000 (default);
  // fractional ranging is not enabled
  uint16_t range = readReg16Bit(0x14 + 10);

  writeReg(0x0B, 0x01);

  return range;
}


bool getSpadInfo(uint8_t * count, bool * type_is_aperture)
{
  uint8_t tmp;

  writeReg(0x80, 0x01);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x00);

  writeReg(0xFF, 0x06);
  writeReg(0x83, readReg(0x83) | 0x04);
  writeReg(0xFF, 0x07);
  writeReg(0x81, 0x01);

  writeReg(0x80, 0x01);

  writeReg(0x94, 0x6b);
  writeReg(0x83, 0x00);
  //startTimeout();
  while (readReg(0x83) == 0x00)
  {
    //if (checkTimeoutExpired()) { return false; }
  }
  writeReg(0x83, 0x01);
  tmp = readReg(0x92);

  *count = tmp & 0x7f;
  *type_is_aperture = (tmp >> 7) & 0x01;

  writeReg(0x81, 0x00);
  writeReg(0xFF, 0x06);
  writeReg(0x83, readReg(0x83)  & ~0x04);
  writeReg(0xFF, 0x01);
  writeReg(0x00, 0x01);

  writeReg(0xFF, 0x00);
  writeReg(0x80, 0x00);

  return true;
}

bool performSingleRefCalibration(uint8_t vhv_init_byte)
{
  writeReg(0x00, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP

  //startTimeout();
  while ((0x13 & 0x07) == 0)
  {
    //if (checkTimeoutExpired()) { return false; }
  }

  writeReg(0x0B, 0x01);

  writeReg(0x00, 0x00);

  return true;
}

uint16_t encodeTimeout(uint16_t timeout_mclks)
{
  // format: "(LSByte * 2^MSByte) + 1"

  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0)
  {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0)
    {
      ls_byte >>= 1;
      ms_byte++;
    }

    return (ms_byte << 8) | (ls_byte & 0xFF);
  }
  else { return 0; }
}

uint16_t decodeTimeout(uint16_t reg_val)
{
  // format: "(LSByte * 2^MSByte) + 1"
  return (uint16_t)((reg_val & 0x00FF) <<
         (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

  return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
  uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

  return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

    struct SequenceStepEnables
    {
      bool tcc, msrc, dss, pre_range, final_range;
    };

    struct SequenceStepTimeouts
    {
      uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

      uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
      uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
    };

uint8_t getVcselPulsePeriod(enum vcselPeriodType type)
{
  if (type == VcselPeriodPreRange)
  {
    return decodeVcselPeriod(readReg(0x50));
  }
  else if (type == VcselPeriodFinalRange)
  {
    return decodeVcselPeriod(readReg(0x70));
  }
  else { return 255; }
}

void getSequenceStepEnables(struct SequenceStepEnables * enables)
{
  uint8_t sequence_config = readReg(0x01);

  enables->tcc          = (sequence_config >> 4) & 0x1;
  enables->dss          = (sequence_config >> 3) & 0x1;
  enables->msrc         = (sequence_config >> 2) & 0x1;
  enables->pre_range    = (sequence_config >> 6) & 0x1;
  enables->final_range  = (sequence_config >> 7) & 0x1;
}

void getSequenceStepTimeouts(struct SequenceStepEnables const * enables, struct SequenceStepTimeouts * timeouts)
{
  timeouts->pre_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodPreRange);

  timeouts->msrc_dss_tcc_mclks = readReg(0x46) + 1;
  timeouts->msrc_dss_tcc_us =
    timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->pre_range_mclks =
    decodeTimeout(readReg16Bit(0x71));
  timeouts->pre_range_us =
    timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->final_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodFinalRange);

  timeouts->final_range_mclks =
    decodeTimeout(readReg16Bit(0x71));

  if (enables->pre_range)
  {
    timeouts->final_range_mclks -= timeouts->pre_range_mclks;
  }

  timeouts->final_range_us =
    timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                               timeouts->final_range_vcsel_period_pclks);
}

uint32_t getMeasurementTimingBudget(void)
{
  struct SequenceStepEnables enables;
  struct SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  // "Start and end overhead times always present"
  uint32_t budget_us = StartOverhead + EndOverhead;

  getSequenceStepEnables(&enables);
  getSequenceStepTimeouts(&enables, &timeouts);

  if (enables.tcc)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss)
  {
    budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc)
  {
    budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range)
  {
    budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range)
  {
    budget_us += (timeouts.final_range_us + FinalRangeOverhead);
  }

  measurement_timing_budget_us = budget_us; // store for internal reuse
  return budget_us;
}

bool setMeasurementTimingBudget(uint32_t budget_us)
{
  struct SequenceStepEnables enables;
  struct SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  uint32_t const MinTimingBudget = 20000;

  if (budget_us < MinTimingBudget) { return false; }

  uint32_t used_budget_us = StartOverhead + EndOverhead;

  getSequenceStepEnables(&enables);
  getSequenceStepTimeouts(&enables, &timeouts);

  if (enables.tcc)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss)
  {
    used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc)
  {
    used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range)
  {
    used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range)
  {
    used_budget_us += FinalRangeOverhead;

    // "Note that the final range timeout is determined by the timing
    // budget and the sum of all other timeouts within the sequence.
    // If there is no room for the final range timeout, then an error
    // will be set. Otherwise the remaining time will be applied to
    // the final range."

    if (used_budget_us > budget_us)
    {
      // "Requested timeout too big."
      return false;
    }

    uint32_t final_range_timeout_us = budget_us - used_budget_us;

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

    // "For the final range timeout, the pre-range timeout
    //  must be added. To do this both final and pre-range
    //  timeouts must be expressed in macro periods MClks
    //  because they have different vcsel periods."

    uint16_t final_range_timeout_mclks =
      timeoutMicrosecondsToMclks(final_range_timeout_us,
                                 timeouts.final_range_vcsel_period_pclks);

    if (enables.pre_range)
    {
      final_range_timeout_mclks += timeouts.pre_range_mclks;
    }

    writeReg16Bit(0x71,
      encodeTimeout(final_range_timeout_mclks));

    // set_sequence_step_timeout() end

    measurement_timing_budget_us = budget_us; // store for internal reuse
  }
  return true;
}

bool initSensor(void) {
	// "Set I2C standard mode"
  	writeReg(0x88, 0x00);

  	writeReg(0x80, 0x01);
  	writeReg(0xFF, 0x01);
  	writeReg(0x00, 0x00);
  	stop_variable = readReg(0x91);
  	writeReg(0x00, 0x01);
  	writeReg(0xFF, 0x00);
  	writeReg(0x80, 0x00);

  	// disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
  	writeReg(0x60, readReg(0x60) | 0x12);

  	// set final range signal rate limit to 0.25 MCPS (million counts per second)
  	//	setSignalRateLimit(0.25);

  	writeReg(0x01, 0xFF);

  	// VL53L0X_DataInit() end

  	// VL53L0X_StaticInit() begin

  	uint8_t spad_count;
  	bool spad_type_is_aperture;
  	if (!getSpadInfo(&spad_count, &spad_type_is_aperture)) { return false; }

  	// The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
  	// the API, but the same data seems to be more easily readable from
  	// GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
  	uint8_t ref_spad_map[6];
  	//readMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

  	// -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)

  	writeReg(0xFF, 0x01);
  	writeReg(0x4F, 0x00);
  	writeReg(0x4E, 0x2C);
  	writeReg(0xFF, 0x00);
  	writeReg(0xB6, 0xB4);

  	uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
  	uint8_t spads_enabled = 0;

  	for (uint8_t i = 0; i < 48; i++)
  	{
    		if (i < first_spad_to_enable || spads_enabled == spad_count)
    		{
      			// This bit is lower than the first one that should be enabled, or
      			// (reference_spad_count) bits have already been enabled, so zero this bit
      			ref_spad_map[i / 8] &= ~(1 << (i % 8));
    		}
    		else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
    		{
      			spads_enabled++;
    		}
  	}

  	writeMulti(0xB0, ref_spad_map, 6);

  	// -- VL53L0X_set_reference_spads() end

  	// -- VL53L0X_load_tuning_settings() begin
  	// DefaultTuningSettings from vl53l0x_tuning.h

  	writeReg(0xFF, 0x01);
  	writeReg(0x00, 0x00);

  	writeReg(0xFF, 0x00);
  	writeReg(0x09, 0x00);
  	writeReg(0x10, 0x00);
  	writeReg(0x11, 0x00);

  	writeReg(0x24, 0x01);
  	writeReg(0x25, 0xFF);
  	writeReg(0x75, 0x00);

  	writeReg(0xFF, 0x01);
  	writeReg(0x4E, 0x2C);
  	writeReg(0x48, 0x00);
  	writeReg(0x30, 0x20);

  	writeReg(0xFF, 0x00);
  	writeReg(0x30, 0x09);
  	writeReg(0x54, 0x00);
  	writeReg(0x31, 0x04);
  	writeReg(0x32, 0x03);
  	writeReg(0x40, 0x83);
  	writeReg(0x46, 0x25);
  	writeReg(0x60, 0x00);
  	writeReg(0x27, 0x00);
  	writeReg(0x50, 0x06);
  	writeReg(0x51, 0x00);
  	writeReg(0x52, 0x96);
  	writeReg(0x56, 0x08);
  	writeReg(0x57, 0x30);
  	writeReg(0x61, 0x00);
  	writeReg(0x62, 0x00);
  	writeReg(0x64, 0x00);
  	writeReg(0x65, 0x00);
  	writeReg(0x66, 0xA0);

  	writeReg(0xFF, 0x01);
  	writeReg(0x22, 0x32);
  	writeReg(0x47, 0x14);
  	writeReg(0x49, 0xFF);
  	writeReg(0x4A, 0x00);

  	writeReg(0xFF, 0x00);
  	writeReg(0x7A, 0x0A);
  	writeReg(0x7B, 0x00);
  	writeReg(0x78, 0x21);

  	writeReg(0xFF, 0x01);
  	writeReg(0x23, 0x34);
  	writeReg(0x42, 0x00);
  	writeReg(0x44, 0xFF);
  	writeReg(0x45, 0x26);
  	writeReg(0x46, 0x05);
  	writeReg(0x40, 0x40);
  	writeReg(0x0E, 0x06);
  	writeReg(0x20, 0x1A);
  	writeReg(0x43, 0x40);

  	writeReg(0xFF, 0x00);
  	writeReg(0x34, 0x03);
  	writeReg(0x35, 0x44);

  	writeReg(0xFF, 0x01);
  	writeReg(0x31, 0x04);
  	writeReg(0x4B, 0x09);
  	writeReg(0x4C, 0x05);
  	writeReg(0x4D, 0x04);

  	writeReg(0xFF, 0x00);
  	writeReg(0x44, 0x00);
  	writeReg(0x45, 0x20);
  	writeReg(0x47, 0x08);
  	writeReg(0x48, 0x28);
  	writeReg(0x67, 0x00);
  	writeReg(0x70, 0x04);
  	writeReg(0x71, 0x01);
  	writeReg(0x72, 0xFE);
  	writeReg(0x76, 0x00);
  	writeReg(0x77, 0x00);

  	writeReg(0xFF, 0x01);
  	writeReg(0x0D, 0x01);

  	writeReg(0xFF, 0x00);
  	writeReg(0x80, 0x01);
  	writeReg(0x01, 0xF8);

  	writeReg(0xFF, 0x01);
  	writeReg(0x8E, 0x01);
  	writeReg(0x00, 0x01);
  	writeReg(0xFF, 0x00);
  	writeReg(0x80, 0x00);

  	// -- VL53L0X_load_tuning_settings() end

  	// "Set interrupt config to new sample ready"
  	// -- VL53L0X_SetGpioConfig() begin

  	writeReg(0x0A, 0x04);
  	writeReg(0x84, readReg(0x84) & ~0x10); // active low
  	writeReg(0x0B, 0x01);

  	// -- VL53L0X_SetGpioConfig() end

  	measurement_timing_budget_us = getMeasurementTimingBudget();

  	// "Disable MSRC and TCC by default"
  	// MSRC = Minimum Signal Rate Check
  	// TCC = Target CentreCheck
  	// -- VL53L0X_SetSequenceStepEnable() begin

  	writeReg(0x01, 0xE8);

  	// -- VL53L0X_SetSequenceStepEnable() end

  	// "Recalculate timing budget"
  	//setMeasurementTimingBudget(measurement_timing_budget_us);

  	// VL53L0X_StaticInit() end

  	// VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())

  	// -- VL53L0X_perform_vhv_calibration() begin

  	writeReg(0x01, 0x01);
  	if (!performSingleRefCalibration(0x40)) { return false; }

  	// -- VL53L0X_perform_vhv_calibration() end

  	// -- VL53L0X_perform_phase_calibration() begin

  	writeReg(0x01, 0x02);
  	if (!performSingleRefCalibration(0x00)) { return false; }

  	// -- VL53L0X_perform_phase_calibration() end

  	// "restore the previous Sequence Config"
  	writeReg(0x01, 0xE8);

	return true;
}

int main()
{
	char msg[20];
	int dist = 0;

	printf("initSensor returned %d\n", initSensor());

	init();
	fillRect(0,0,WIDTH,HEIGHT,0);

	startContinuous(0);

	for (uint8_t i = 0;; i++)
	{
		icosoc_leds(i);

		dist = readRangeContinuousMillimeters();
		sprintf(msg, "Distance: %5d", dist);
		drawText(10,20,msg,0xFFFF);
		
		for (int i = 0; i < 1000000; i++)
			asm volatile ("");
	}
}

