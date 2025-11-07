/**
 * @file
 * @brief Definitions of the clock library
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
 *
 * @brief Definitions of the clock library
 * @details
 * Definitions of the generic functions for calculating clock error over temperature.
 */

#ifndef CONV_CLOCK_ERROR_H_
    #define CONV_CLOCK_ERROR_H_

#include <stdint.h>

/** Initialize the conversion module
 *
 * This function will read the adc offset(s) and gain(s) from
 * Melexis calibration EEPROM area and store it in RAM for fast usage
 * in calls to `conv_clock_error_speed`.
 */
void conv_clock_error_init(void);

/** System clock error at given temperature
 *
 * @param[in]  raw_temperature  chip temperature raw adc measurement.
 * @return  clock frequency difference vs. nominal, scaled by 1/2^11.
 */
int16_t conv_clock_error_speed(uint16_t raw_temperature);

/** Correct a period with a given system clock error
 *
 * @param[in]  period  period to correct.
 * @param[in]  clock_error  clock error as reported by conv_clock_error_speed().
 * @return  corrected period.
 */
uint16_t conv_clock_error_correct_period(uint16_t period, int16_t clock_error);

#endif  /* CONV_CLOCK_ERROR_H_ */

/* EOF */

