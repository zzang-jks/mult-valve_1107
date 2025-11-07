/**
 * @file
 * @brief Low voltage conversion function for the adc module
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
 * @details Low voltage conversion function for the adc module, this will convert adc measurements
 * into SI units.
 */
#include <compiler_abstraction.h>
#include <mathlib.h>
#include <eeprom_map.h>
#include "lib_adc_conv_private.h"
#include "conv_low_voltage.h"

/** low voltage input calibration data */
calib_data_t calib_lvi;

void conv_low_voltage_init(void)
{
#if defined(EE_O_LVI_GET)
    calib_lvi.simple.offset = EE_GET(O_LVI);
    calib_lvi.simple.gain = EE_GET(GAIN_LVI);
    calib_lvi.gain_low_t = EE_GET(GAINLO_LVI);
    calib_lvi.gain_high_t = EE_GET(GAINHI_LVI);
#else
    calib_lvi.simple.offset = 0;
    calib_lvi.simple.gain = 213u;                   /* 2.5V * 1.36 * 1000 * LVI_GAIN_DIV / 1023 */
    calib_lvi.gain_low_t = 0;
    calib_lvi.gain_high_t = 0;
#endif

    if (calib_lvi.simple.gain == 0u)
    {
        /* gain 0 is not allowed, use the default value */
        calib_lvi.simple.gain = 213u;               /* 2.5V * 1.36 * 1000 * LVI_GAIN_DIV / 1023 */
    }
}

int16_t conv_low_voltage(uint16_t u16ADC_Value)
{
    /* cLVI = (mLVI - O_LVI_EE) * Gain_LVI_EE / 2^6 */
    return apply_offset_gain(u16ADC_Value, &calib_lvi.simple, LVI_GAIN_DIV);
}

int16_t conv_low_voltage_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp)
{
    /* mTempDiff = mTemp - OTempCal_EE
     * if(mTempDiff > 0){
     *     tempGain = ((GainLo_LVI_EE / 2^11) * mTempDiff)/2^7;
     * }else{
     *     tempGain = ((GainHi_LVI_EE / 2^11) * mTempDiff)/2^7;
     * }
     * cLVIcorr = cLVI * (1 + tempGain)
     */
    return apply_temp_gain(conv_low_voltage(u16ADC_Value), u16ADC_Temp, &calib_lvi);
}
