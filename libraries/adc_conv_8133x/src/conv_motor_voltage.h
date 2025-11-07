/**
 * @file
 * @brief Definitions of the motor voltage adc conversion function
 * @internal
 *
 * @copyright (C) 2020 Melexis N.V.
 *
 * Melexis N.V. is supplying this code for use with Melexis N.V. processor based microcontrollers only.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.  MELEXIS N.V. SHALL NOT IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * @endinternal
 *
 * @ingroup adc
 *
 * @details Definitions of the motor voltage conversion function for the adc module, this will convert
 * adc measurements into SI units.
 */

#ifndef CONV_MOTOR_VOLTAGE_H_
    #define CONV_MOTOR_VOLTAGE_H_

#include <stdint.h>

/** Initialize the adc conversion module */
void conv_motor_voltage_init(void);

/** Convert adc bits to motor voltage
 *
 * Conversion function for motor voltage input.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @return  converted voltage in 10 mV.
 */
int16_t conv_motor_voltage(uint16_t u16ADC_Value);

/** Convert adc bits to motor voltage
 *
 * Conversion function for motor voltage input.
 * This function is the same as `conv_motor_voltage` but including correction for
 * ic temperature deviations.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @param[in]  u16ADC_Temp  Raw adc temperature measurement.
 * @return  converted voltage in 10 mV.
 */
int16_t conv_motor_voltage_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp);

/** Convert adc bits to motor voltage filtered
 *
 * Conversion function for motor voltage filtered input.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @return  converted voltage in 10 mV.
 */
int16_t conv_motor_voltage_filtered(uint16_t u16ADC_Value);

/** Convert motor voltage filtered to adc bits
 *
 * Conversion function for motor voltage filtered input.
 * @param[in]  i16VSMF  voltage in 10 mV.
 * @return  Converted raw adc measurement.
 */
uint16_t conv_motor_voltage_filtered_to_adc(int16_t i16VSMF);

/** Convert adc bits to motor voltage filtered (Vsmf)
 *
 * Conversion function for motor voltage filtered input.
 * This function is the same as `conv_motor_voltage_filtered` but including correction for
 * ic temperature deviations.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @param[in]  u16ADC_Temp  Raw adc temperature measurement.
 * @return  converted voltage in 10 mV.
 */
int16_t conv_motor_voltage_filtered_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp);

#endif /* CONV_MOTOR_VOLTAGE_H_ */
