/**
 * @file
 * @brief The pwm routines definitions.
 * @internal
 *
 * @copyright (C) 2018-2020 Melexis N.V.
 *
 * Revision 0.6.0
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
 * @details This file contains the definitions of the pwm driver module.
 */

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>
#include <stdbool.h>
#include <compiler_abstraction.h>
#include "defines.h"
/* ---------------------------------------------
 * Public Defines
 * --------------------------------------------- */

/* ---------------------------------------------
 * Public Enumerations
 * --------------------------------------------- */
/** pwm channel selection enum */
typedef enum
{
    eCHU = 1,    /**< select U pwm channel */
    eCHV = 2,    /**< select V pwm channel */
    eCHW = 4,    /**< select W pwm channel */
    eCHT = 8,    /**< select T pwm channel */
    eAll = 0xF   /**< select all pwm channel */
} pwm_Channel_t; /**< pwm channel selection type */

/* ---------------------------------------------
 * Public Variables
 * --------------------------------------------- */

/* ---------------------------------------------
 * Public Function Declarations
 * --------------------------------------------- */
void pwm_Init(void);
void pwm_Start(uint8_t dir, uint16_t u16DutyCycle);
void pwm_SetDutyCycle(uint8_t dir, uint16_t u16DutyCycle);
void pwm_SetMaxDutyCycle(uint16_t u16DutyCycle);
void pwm_Stop(void);
void pwm_Off(void);
void pwm_Disable(void);

#endif /* PWM_H_ */
