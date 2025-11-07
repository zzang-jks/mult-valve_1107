/**
 * @file
 * @brief Clock library
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
 * @ingroup clock
 *
 * @details This file contains the implementations of the clock library.
 * @{
 */

#include <stdint.h>
#include "eeprom_map.h"
#include "mathlib.h"
#include "conv_clock_error.h"

int16_t clock_error_OTempCal;  /**< Raw ADC measurement of the temperature sensor at mid test temperature */
int16_t clock_error_SClockLowCal;  /**< CPU clock gain for low temperature range */
int16_t clock_error_SClockHighCal;  /**< CPU clock gain for high temperature range */
int16_t clock_error_OClockCal;  /**< CPU clock offset */

void conv_clock_error_init(void)
{
    clock_error_OTempCal = EE_GET(OTEMPCAL);

    #if (FPLL == 32000) || (FPLL == 16000)
    clock_error_SClockLowCal = EE_GET(SCLOCK32LOWCAL);
    clock_error_SClockHighCal = EE_GET(SCLOCK32HIGHCAL);
    clock_error_OClockCal = EE_GET(OCLOCK32CAL);
    #elif (FPLL == 28000) || (FPLL == 14000)
    clock_error_SClockLowCal = EE_GET(SCLOCK28LOWCAL);
    clock_error_SClockHighCal = EE_GET(SCLOCK28HIGHCAL);
    clock_error_OClockCal = EE_GET(OCLOCK28CAL);
    #elif (FPLL == 24000) || (FPLL == 12000)
    clock_error_SClockLowCal = EE_GET(SCLOCK24LOWCAL);
    clock_error_SClockHighCal = EE_GET(SCLOCK24HIGHCAL);
    clock_error_OClockCal = EE_GET(OCLOCK24CAL);
    #else
    clock_error_SClockLowCal = 0;
    clock_error_SClockHighCal = 0;
    clock_error_OClockCal = 0;
    #endif
}

int16_t conv_clock_error_speed(uint16_t raw_temperature)
{
    int16_t temp;
    int16_t gain;

    temp = (int16_t)raw_temperature - clock_error_OTempCal;

    if (temp > 0)
    {
        gain = clock_error_SClockLowCal;
    }
    else
    {
        gain = clock_error_SClockHighCal;
    }

    return clock_error_OClockCal + (int16_t)(mulI32_I16byI16(temp, gain) / 64);
}

uint16_t conv_clock_error_correct_period(uint16_t period, int16_t clock_error)
{
    int16_t temp = 2048 + clock_error;
    return (uint16_t)(mulU32_U16byU16(period, (uint16_t)temp) / 2048u);
}

/* EOF */

