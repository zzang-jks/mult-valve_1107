/**
 * @file
 * @brief Private definitions of the generic adc conversion functions
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
 * @details Private definitions of the generic conversion function for the adc module.
 */

#ifndef LIB_ADC_CONV_PRIVATE_H_
    #define LIB_ADC_CONV_PRIVATE_H_

#include <stdint.h>
#include "lib_adc_conv.h"

/** Divider for the voltage gain factor */
#define HVI_GAIN_DIV 32

/** Divider for the voltage gain factor */
#define LVI_GAIN_DIV 64

/** Divider for the current gain factor */
#ifdef MLX81330
#define CURR_GAIN_DIV  64
#else
#ifdef MLX81332
#define CURR_GAIN_DIV  32
#else
#ifdef MLX81334
#define CURR_GAIN_DIV  32
#else
#error Product is not supported by this library.
#endif
#endif
#endif

/** Apply offset and gain to a measurement
 *
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @param[in]  cal_data  calibration data to be used in calculations.
 * @param[in]  divider  gain divider factor to be used.
 * @return  updated measurement.
 */
STATIC INLINE int16_t apply_offset_gain(uint16_t u16ADC_Value, simple_calib_data_t * cal_data, uint8_t divider)
{
    int16_t measurement = (int16_t)u16ADC_Value;
    measurement -= cal_data->offset;
    measurement = (int16_t)(mulI24_I16byU8(measurement, cal_data->gain) / divider);
    return (measurement);
}

/** Apply temperature dependent gain to a measurement
 *
 * @param[in]  u16ADC_Value  Converted adc measurement.
 * @param[in]  u16ADC_Temp  Raw adc temperature measurement.
 * @param[in]  cal_data  calibration data to be used in calculations.
 * @return  updated measurement.
 */
STATIC INLINE int16_t apply_temp_gain(uint16_t u16ADC_Value, uint16_t u16ADC_Temp, calib_data_t * cal_data)
{
    int16_t gain;
    int16_t temp_diff = (int16_t)(u16ADC_Temp - EE_GET(OTEMPCAL));

    if (temp_diff > 0)
    {
        gain = (int16_t)(mulI24_I16byI8(temp_diff, cal_data->gain_low_t) / 128);
    }
    else
    {
        gain = (int16_t)(mulI24_I16byI8(temp_diff, cal_data->gain_high_t) / 128);
    }

    return (u16ADC_Value + (mulI32_I16byI16(gain, u16ADC_Value) / 2048));
}

#endif /* LIB_ADC_CONV_PRIVATE_H_ */
