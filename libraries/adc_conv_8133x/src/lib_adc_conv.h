/**
 * @file
 * @brief Definitions of the generic adc conversion function
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
 * @details Definitions of the generic conversion function for the adc module, this will convert adc
 * measurements into SI units.
 */

#ifndef LIB_ADC_CONV_H_
    #define LIB_ADC_CONV_H_

#include <stdint.h>

/** simple calibration data struct */
typedef struct simple_calib_data_s
{
    int16_t offset;                                 /**< offset */
    uint16_t gain;                                  /**< gain */
} simple_calib_data_t;                              /**< simple calibration data type */

/** calibration data struct */
typedef struct calib_data_s
{
    simple_calib_data_t simple;                     /**< simple calibration data */
    int8_t gain_low_t;                              /**< extra gain for low temperature */
    int8_t gain_high_t;                             /**< extra gain for high temperature */
} calib_data_t;                                     /**< calibration data type */

#endif /* LIB_ADC_CONV_H_ */
