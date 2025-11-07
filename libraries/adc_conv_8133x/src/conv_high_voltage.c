/**
 * @file
 * @brief High voltage conversion function for the adc module
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
 * @details High voltage conversion function for the adc module, this will convert adc measurements
 * into SI units.
 */
#include <compiler_abstraction.h>
#include <mathlib.h>
#include <eeprom_map.h>
#include "lib_adc_conv_private.h"
#include "conv_high_voltage.h"

/** high voltage input calibration data */
calib_data_t calib_hvi;

void conv_high_voltage_init(void)
{
#if defined(EE_O_HVI_GET)
    calib_hvi.simple.offset = EE_GET(O_HVI);
    calib_hvi.simple.gain = EE_GET(GAIN_HVI);
    calib_hvi.gain_low_t = EE_GET(GAINLO_VSMF);
    calib_hvi.gain_high_t = EE_GET(GAINHI_VSMF);
#else
    calib_hvi.simple.offset = EE_GET(VS_OFFS);
    calib_hvi.simple.gain = EE_GET(VS_GAIN);
    calib_hvi.gain_low_t = 0;
    calib_hvi.gain_high_t = 0;
#endif

    if (calib_hvi.simple.gain == 0u)
    {
        /* gain 0 is not allowed, use the default value */
        calib_hvi.simple.gain = 164u;               /* 2.5V * 21 * 100 * HVI_GAIN_DIV / 1023 */
    }
}

int16_t conv_high_voltage(uint16_t u16ADC_Value)
{
    /* cHVI = (mHVI - O_HVI_EE) * Gain_HVI_EE / 2^5 */
    return apply_offset_gain(u16ADC_Value, &calib_hvi.simple, HVI_GAIN_DIV);
}

int16_t conv_high_voltage_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp)
{
    /* mTempDiff = mTemp - OTempCal_EE
     * if(mTempDiff > 0){
     *     tempGain = ((GainLo_VSMF_EE / 2^11) * mTempDiff)/2^7;
     * }else{
     *     tempGain = ((GainHi_VSMF_EE / 2^11) * mTempDiff)/2^7;
     * }
     * cHVIcorr = cHVI * (1 + tempGain)
     */
    return apply_temp_gain(conv_high_voltage(u16ADC_Value), u16ADC_Temp, &calib_hvi);
}
