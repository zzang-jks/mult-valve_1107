/**
 * @file
 * @brief application configuration of the swtimer library
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
 * @ingroup application
 *
 * @details This file configuration of the software timer library.
 */

#ifndef SWTIMER_CONFIG_H_
#define SWTIMER_CONFIG_H_

#include <timerlib.h>

/** Timer mode */
#define CT_MODE STIMER_1US_CLOCK

/** timer period [us] */
#define CT_PERIODIC_RATE (100U) /*100usec*/

/** software timer identifiers */
typedef enum swtimer_id
{
    SWTIMER_CHECKCOLIN, /**< lin22.c */
    SWTIMER_LINAS,      /**< lin22.c */
    SWTIMER_LINCR,      /**< lin22.c */
    SWTIMER_PROTECTION_DELAY_VSM,
    SWTIMER_PROTECTION_DELAY_OVT,
    SWTIMER_MOT_CTRL_PERIOD, /**< motor 500usec */
    SWTIMER_APP_CTRL_PERIOD, /* 1msec */

    SWTIMER_NR_OF /**< Number of application software timers */
} swtimer_id_t;   /**< software timer identifiers type */

#endif /* SWTIMER_CONFIG_H_ */

/* EOF */
