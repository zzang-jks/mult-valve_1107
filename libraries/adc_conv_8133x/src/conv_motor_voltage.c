/**
 * @file
 * @brief Motor voltage conversion function for the adc module
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
 * @details Motor voltage conversion function for the adc module, this will convert adc measurements
 * into SI units.
 */
#include <compiler_abstraction.h>
#include <mathlib.h>
#include <eeprom_map.h>
#include "lib_adc_conv_private.h"
#include "conv_motor_voltage.h"

/** VSM calibration data */
calib_data_t calib_vsm;
/** VSM filtered calibration data */
calib_data_t calib_vsmf;

void conv_motor_voltage_init(void)
{
#if defined(EE_O_HVI_GET)
    calib_vsm.simple.offset = EE_GET(O_HVI);
    calib_vsm.simple.gain = EE_GET(GAIN_VSMF);
    calib_vsm.gain_low_t = EE_GET(GAINLO_VSMF);
    calib_vsm.gain_high_t = EE_GET(GAINHI_VSMF);
#else
    calib_vsm.simple.offset = EE_GET(VSM_OFFS);
    calib_vsm.simple.gain = EE_GET(VSM_GAIN);
    calib_vsm.gain_low_t = 0;
    calib_vsm.gain_high_t = 0;
#endif

    if (calib_vsm.simple.gain == 0u)
    {
        /* gain 0 is not allowed, use the default value */
        calib_vsm.simple.gain = 164u;               /* 2.5V * 21 * 100 * HVI_GAIN_DIV / 1023 */
    }

#if defined(EE_O_HVI_GET)
    calib_vsmf.simple.offset = EE_GET(O_HVI) + EE_GET(O_VSMF);
    calib_vsmf.simple.gain = EE_GET(GAIN_VSMF);
    calib_vsmf.gain_low_t = EE_GET(GAINLO_VSMF);
    calib_vsmf.gain_high_t = EE_GET(GAINHI_VSMF);
#else
    calib_vsmf.simple.offset = EE_GET(VSM_FILT_OFFS);
    calib_vsmf.simple.gain = EE_GET(VSM_FILT_GAIN);
    calib_vsmf.gain_low_t = 0;
    calib_vsmf.gain_high_t = 0;
#endif

    if (calib_vsmf.simple.gain == 0u)
    {
        /* gain 0 is not allowed, use the default value */
        calib_vsmf.simple.gain = 164u;              /* 2.5V * 21 * 100 * HVI_GAIN_DIV / 1023 */
    }
}

int16_t conv_motor_voltage(uint16_t u16ADC_Value)
{
    /* cVSM = (mVSM - O_HVI_EE) * Gain_VSMF_EE / 2^5 */
    return apply_offset_gain(u16ADC_Value, &calib_vsm.simple, HVI_GAIN_DIV);
}

int16_t conv_motor_voltage_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp)
{
    /* mTempDiff = mTemp - OTempCal_EE
     * if(mTempDiff > 0){
     *     tempGain = ((GainLo_VSMF_EE / 2^11) * mTempDiff)/2^7;
     * }else{
     *     tempGain = ((GainHi_VSMF_EE / 2^11) * mTempDiff)/2^7;
     * }
     * cVSMcorr = cVSM * (1 + tempGain)
     */
    return apply_temp_gain(conv_motor_voltage(u16ADC_Value), u16ADC_Temp, &calib_vsm);
}

int16_t conv_motor_voltage_filtered(uint16_t u16ADC_Value)
{
    /* cVSMF = (mVSMF - O_HVI_EE - O_VSMF_EE) * Gain_VSMF_EE / 2^5 */
    return apply_offset_gain(u16ADC_Value, &calib_vsmf.simple, HVI_GAIN_DIV);
}

uint16_t conv_motor_voltage_filtered_to_adc(int16_t i16VSMF)
{
    uint16_t u16ADC_Value;

    uint16_t gain = calib_vsmf.simple.gain;
    u16ADC_Value = divU16_U32byU16(1UL * i16VSMF * HVI_GAIN_DIV, gain);
    u16ADC_Value += calib_vsmf.simple.offset;

    return u16ADC_Value;
}

int16_t conv_motor_voltage_filtered_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp)
{
    /* mTempDiff = mTemp - OTempCal_EE
     * if(mTempDiff > 0){
     *     tempGain = ((GainLo_VSMF_EE / 2^11) * mTempDiff)/2^7;
     * }else{
     *     tempGain = ((GainHi_VSMF_EE / 2^11) * mTempDiff)/2^7;
     * }
     * cVSMFcorr = cVSMF * (1 + tempGain)
     */
    return apply_temp_gain(conv_motor_voltage_filtered(u16ADC_Value), u16ADC_Temp, &calib_vsmf);
}
