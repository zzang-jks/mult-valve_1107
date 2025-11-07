/**
 * @file
 * @brief Definitions of the shunt current adc conversion function
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
 * @details Definitions of the shunt current conversion function for the adc module, this will convert
 * adc measurements into SI units.
 */

#ifndef CONV_SHUNT_CURRENT_INLINE_IMPL_H_
    #define CONV_SHUNT_CURRENT_INLINE_IMPL_H_

STATIC INLINE uint16_t conv_get_shunt_current_offset(void)
{
    extern calib_data_t calib_current;
    return calib_current.simple.offset;
}

STATIC INLINE void conv_update_shunt_current_offset(uint16_t value)
{
    extern calib_data_t calib_current;
    calib_current.simple.offset = value;
}

#endif /* CONV_SHUNT_CURRENT_INLINE_IMPL_H_ */
