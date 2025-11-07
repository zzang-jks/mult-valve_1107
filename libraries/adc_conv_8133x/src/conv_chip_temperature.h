/**
 * @file
 * @brief ADC sample to temperature conversion library
 * @internal
 *
 * @copyright (C) 2022 Melexis N.V.
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
 * @ingroup libraries
 * @addtogroup calculate_temperature Calculate Temperature
 *
 * @brief ADC sample to temperature conversion library
 * @details
 * Definitions of the generic functions for calculating temperature for ADC values.
 */

#ifndef LIB_CONV_CHIP_TEMPERATURE_H_
#define LIB_CONV_CHIP_TEMPERATURE_H_

#include <stdint.h>

/** Initialize the conversion module
 *
 * This function will read the adc offset(s) and gain(s) from
 * Melexis calibration EEPROM area and store it in RAM for fast usage
 * in calls to `conv_chip_temperature`.
 */
void conv_chip_temperature_init(void);

/** Convert adc bits to chip temperature (via 3 point linearization)
 *
 * When convering the ADC bits to the chip temperature.
 * TEMPMID will be used unless it is 0 then TEMPMID_CP will be used instead.
 * @param[in]  adcValue  Raw adc measurement.
 * @return  int16_t  the temperature in 1 degC.
 */
int16_t conv_chip_temperature(uint16_t adcValue);

#endif /* LIB_CONV_CHIP_TEMPERATURE_H_ */

/* EOF */
