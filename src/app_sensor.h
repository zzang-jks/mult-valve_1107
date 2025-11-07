/*
 * app_sensor.h
 *
 *  Created on: 2025. 5. 29.
 *      Author: mctp
 */

#ifndef CODE_SRC_DCM_SENSOR_H_
#define CODE_SRC_DCM_SENSOR_H_
#include <stdint.h>
#include "defines.h"
#define C_ACTR_POS_MAX (1 << 12)
#define C_SENSOR_SUPPLY_VREF (float)((3.3 - 2.5) / 1.36) // 752u	//0x2F0

#define C_FBS_FULL_ANGLE C_ACTR_POS_MAX
#define REVERSE_FBS_ENABLE

#define C_GMR_OUTPUT_OFFSET 0

#define C_GMR_POSITIVE_MAX (int16_t)0x3FFFu
#define C_GMR_NEGAITIVE_MAX (int16_t)(-0x3FFFu)

enum
{
  C_ADC_VS_ = 0,
  C_ADC_TEMP_,
  C_ADC_CURRENT_,
  C_ADC_VDDA,
  C_ADC_IGN,
  C_ADC_SENSOR_1,
  C_ADC_SENSOR_2,
  C_ADC_SENSOR_3,
  C_ADC_SENSOR_4,
  C_ADC_END_ITEM
};

typedef struct
{
  uint16_t vs_volt;
  uint16_t ic_temp;
  uint16_t mot_current;
  uint16_t vdda_volt;
  uint16_t ign_volt;
  uint16_t gmr1_in;
  uint16_t gmr2_in;
  uint16_t gmr3_in;
  uint16_t gmr4_in;
} adc_item_t;

typedef struct
{
  uint32_t actual;
  uint32_t correction;
} conversion_t;
void sensor_init(void);
void adc_raw_update(void);
void gmr_calibration_setup(void);
void gmr_calibration_end(void);
void set_gmr_sensor_offset(int16_t offset);
int16_t get_gmr_sensor_offset(void);
int16_t get_sensor_raw_data(uint16_t num);
uint16_t get_conv_vdda_voltage(void);
uint16_t get_conv_supply_voltage(void);
int16_t get_conv_ic_temperature(void);
uint16_t get_conv_mot_current(void);
uint16_t get_conv_ignition_voltage(void);
int16_t get_gmr_sine_output(void);
int16_t get_gmr_cosine_output(void);
int16_t calculate_gmr_angle(void);
int16_t forward_linear_Interpolation(int16_t x, int16_t x0, int16_t x1, int16_t y0, int16_t y1);
int16_t reverse_linear_Interpolation(int16_t x, int16_t x0, int16_t x1, int16_t y0, int16_t y1);
#endif /* CODE_SRC_DCM_SENSOR_H_ */
