/**
 * @file
 * @brief Shunt current conversion function for the adc module
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
 * @details Shunt current conversion function for the adc module, this will convert adc measurements
 * into SI units.
 */
#include <compiler_abstraction.h>
#include <mathlib.h>
#include <eeprom_map.h>
#include "conv_shunt_current.h"

/** current sensor calibration data */
calib_data_t calib_current;

void conv_shunt_current_init(void)
{
#if defined(EE_O_CURR_GET)
    calib_current.simple.offset = EE_GET(O_CURR) + 512;
    calib_current.simple.gain = EE_GET(GAIN_CURR);
    calib_current.gain_low_t = EE_GET(GAINLO_CURR);
    calib_current.gain_high_t = EE_GET(GAINHI_CURR);
#else
    calib_current.simple.offset = EE_GET(CURR_OFFS) + 512;
    calib_current.simple.gain = EE_GET(CURR_GAIN);
    calib_current.gain_low_t = 0;
    calib_current.gain_high_t = 0;
#endif
}

int16_t conv_shunt_current(uint16_t u16ADC_Value)
{
    /* MLX81330: cCur = (mCur - (O_Curr_EE + 512)) * Gain_Curr_EE / 2^6
     * MLX81332: cCur = (mCur - (O_Curr_EE + 512)) * Gain_Curr_EE / 2^5
     */
    return apply_offset_gain(u16ADC_Value, &calib_current.simple, CURR_GAIN_DIV);
}

int16_t conv_shunt_current_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp)
{
    int16_t current = conv_shunt_current(u16ADC_Value);

    /* mTempDiff = mTemp - OTempCal_EE
     * if(mTempDiff > 0){
     *     tempGain = ((GainLo_Curr_EE / 2^10) * mTempDiff)/2^7;
     * }else{
     *     tempGain = ((GainHi_Curr_EE / 2^10) * mTempDiff)/2^7;
     * }
     * cCurrcorr = cCurr * (1 + tempGain)
     */
    int16_t gain;
    int16_t temp_diff = (int16_t)(u16ADC_Temp - EE_GET(OTEMPCAL));

    if (temp_diff > 0)
    {
        gain = (int16_t)(mulI24_I16byI8(temp_diff, calib_current.gain_low_t) / 128);
    }
    else
    {
        gain = (int16_t)(mulI24_I16byI8(temp_diff, calib_current.gain_high_t) / 128);
    }

    return (current + (mulI32_I16byI16(gain, current) / 1024));
}

#ifdef UNITTEST
#include "conv_shunt_current_inline_impl.h"
#endif
