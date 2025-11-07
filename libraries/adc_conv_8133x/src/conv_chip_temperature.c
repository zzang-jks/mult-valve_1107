/**
 * @file
 * @brief Temperature conversion function for the adc module
 * @internal
 *
 * @copyright (C) 2020-2022 Melexis N.V.
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
 * @details Temperature conversion function for the adc module, this will convert adc measurements
 * into SI units.
 */
#include <compiler_abstraction.h>
#include <mathlib.h>
#include <eeprom_map.h>
#include "lib_adc_conv_private.h"
#include "conv_chip_temperature.h"

int8_t mTempMid;  /**< Typical Probing Temperature [C] */
int16_t mTempCal;  /**< Raw temperature ADC measurement at TempMid [LSB] */
uint8_t mTempGainLowCal;  /**< gain factor of temp sensor from TempLow to TempMid [K/LSB] */
uint8_t mTempGainHighCal;  /**< gain factor of temp sensor from TempMid to TempHigh [K/LSB] */


void conv_chip_temperature_init(void)
{
    mTempMid = EE_GET(TEMPMID);
    mTempCal = (int16_t)EE_GET(OTEMPCAL);
    mTempGainLowCal = EE_GET(GAINTEMPLOWCAL);
    mTempGainHighCal = EE_GET(GAINTEMPHIGHCAL);

    if (mTempMid == 0)
    {
        mTempMid = EE_GET(TEMPMID_CP);
    }
}

int16_t conv_chip_temperature(uint16_t adcValue)
{
    uint8_t gain;
    int16_t cTempDiff;
    int16_t mTempDiff = mTempCal - (int16_t)adcValue;

    if (mTempDiff < 0)
    {
        gain = mTempGainLowCal;
    }
    else
    {
        gain = mTempGainHighCal;
    }

    cTempDiff = (int16_t)(mulI24_I16byU8(mTempDiff, gain) / 128);
    return (mTempMid + cTempDiff);
}

/* EOF */
