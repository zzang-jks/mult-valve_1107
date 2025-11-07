/**
 * @file
 * @brief Definitions of the high voltage adc conversion function
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
 * @details Definitions of the high voltage conversion function for the adc module, this will convert
 * adc measurements into SI units.
 */

#ifndef CONV_HIGH_VOLTAGE_H_
    #define CONV_HIGH_VOLTAGE_H_

#include <stdint.h>

/** Initialize the adc conversion module */
void conv_high_voltage_init(void);

/** Convert adc bits to high voltage (Vs, Vph_x, IOx_HV)
 *
 * Conversion function for high voltage inputs with HVI/21 voltage divider.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @return  converted voltage in 10 mV.
 */
int16_t conv_high_voltage(uint16_t u16ADC_Value);

/** Convert adc bits to high voltage (Vs, Vph_x, IOx_HV)
 *
 * Conversion function for high voltage inputs with HVI/21 voltage divider.
 * This function is the same as `conv_high_voltage` but including correction for
 * ic temperature deviations.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @param[in]  u16ADC_Temp  Raw adc temperature measurement.
 * @return  converted voltage in 10 mV.
 */
int16_t conv_high_voltage_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp);

#endif /* CONV_HIGH_VOLTAGE_H_ */
