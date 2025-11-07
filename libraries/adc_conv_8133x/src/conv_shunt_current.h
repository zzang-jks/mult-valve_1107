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

#ifndef CONV_SHUNT_CURRENT_H_
    #define CONV_SHUNT_CURRENT_H_

#include <compiler_abstraction.h>
#include <stdint.h>
#include <eeprom_map.h>
#include "lib_adc_conv_private.h"
#include "lib_adc_conv.h"

/** Initialize the adc conversion module */
void conv_shunt_current_init(void);

/** Convert adc bits to shunt current
 *
 * Conversion function for current shunt measurements.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @return  the shunt current in mA.
 */
int16_t conv_shunt_current(uint16_t u16ADC_Value);

/** Convert adc bits to shunt current
 *
 * Conversion function for current shunt measurements.
 * This function is the same as `conv_shunt_current` but including correction for
 * ic temperature deviations.
 * @param[in]  u16ADC_Value  Raw adc measurement.
 * @param[in]  u16ADC_Temp  Raw adc temperature measurement.
 * @return  the shunt current in mA.
 */
int16_t conv_shunt_current_with_tcorrection(uint16_t u16ADC_Value, uint16_t u16ADC_Temp);

/** Read the currently configured adc offset value
 *
 * @returns  currently configured adc offset.
 */
STATIC INLINE uint16_t conv_get_shunt_current_offset(void);

/** Update the currently configured adc offset value
 *
 * @param[in]  value  new adc offset value to configure.
 */
STATIC INLINE void conv_update_shunt_current_offset(uint16_t value);

#ifndef UNITTEST
#include "conv_shunt_current_inline_impl.h"
#endif

#endif /* CONV_SHUNT_CURRENT_H_ */
