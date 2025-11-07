/**
 * @file
 * @brief main application definitions.
 *
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
 * @ingroup application
 *
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

static INLINE uint16_t get_u16SupplyVoltage(void)
{
    extern uint16_t g_u16SupplyVoltage;
    return g_u16SupplyVoltage;
}

static INLINE int16_t get_i16ChipTemperature(void)
{
    extern int16_t g_i16ChipTemperature;
    return g_i16ChipTemperature;
}

static INLINE uint16_t get_u16MotorSupplyVoltage(void)
{
    extern uint16_t g_u16MotorSupplyVoltage;
    return g_u16MotorSupplyVoltage;
}

#endif /* SYSTEM_H_ */

/* EOF */
