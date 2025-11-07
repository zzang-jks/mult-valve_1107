/**
 * @file
 * @brief The software timer library definitions.
 * @internal
 *
 * @copyright (C) 2019-2021 Melexis N.V.
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
 * @ingroup libraries
 *
 * @brief
 * @details
 * @{
 */

#ifndef SWTIMER_H
    #define SWTIMER_H

#include <stdint.h>
#include <stdbool.h>
#include <compiler_abstraction.h>
#include "swtimer_config.h"

/* ---------------------------
 * Public Defines
 * --------------------------- */

/** Default period of 1000us */
#ifndef CT_PERIODIC_RATE
#define CT_PERIODIC_RATE 1000
#endif

/** Core timer main clock frequency [Hz] */
#define C_CT_CLOCK (1000000UL)

/** Number of ticks per millisecond */
#define PI_TICKS_PER_MILLISECOND (1000U / CT_PERIODIC_RATE)

/** Number of ticks per 10x milliseconds */
#define PI_TICKS_PER_10MILLISECOND (10000U / CT_PERIODIC_RATE)

/** Number of ticks per 100x milliseconds */
#define PI_TICKS_PER_100MILLISECOND (100000U / CT_PERIODIC_RATE)

/** Number of ticks per second */
#define PI_TICKS_PER_SECOND (1000000U / CT_PERIODIC_RATE)


/* ---------------------------
 * Public Types
 * --------------------------- */

/**
 * Definition of the software timer struct
 */
typedef struct swtimer_config_s
{
    uint16_t u15Load     : 15;   /**< Period 0...32767 * CT_PERIODIC_RATE [us] */
    uint16_t bRepetitive : 1;    /**< Mode 1=REPETITIVE or 0=SINGLE_SHOT */
    uint16_t u15Current  : 15;   /**< Current timer value 0...32767 * CT_PERIODIC_RATE [us] */
    uint16_t bTriggered  : 1;    /**< End of period reached */
} swtimer_config_t;  /**< software timer configuration type */

/** Modes */
typedef enum swtimer_mode_e
{
    SINGLE_SHOT = 0u,  /**< Single shot mode, stop at 0 */
    REPETITIVE,  /**< Repetitive mode, infinite */
    SWTIMER_MODE_NR_OF  /**< Number of modes */
} swtimer_mode_t;  /**< Mode type */


/* ---------------------------
 * Public Function Definitions
 * --------------------------- */

void swtimer_init(void);
void swtimer_deinit(void);

void swtimer_calibrate(uint16_t u16ADC_Value);
bool swtimer_register(swtimer_id_t a_e8Id, uint16_t a_u16Load, swtimer_mode_t a_eMode);
void swtimer_start(swtimer_id_t a_e8Id);
void swtimer_stop(swtimer_id_t a_e8Id);
bool swtimer_isStopped(swtimer_id_t a_e8Id);
bool swtimer_isRunning(swtimer_id_t a_e8Id);
bool swtimer_isTriggered(swtimer_id_t a_e8Id);


/* ---------------------------------------------
 * Default weak function definitions
 * --------------------------------------------- */

void swtimer_enterIrq(void);
void swtimer_exitIrq(void);
void swtimer_triggerIrq(uint16_t id);


/* ---------------------------
 * Public Variables
 * --------------------------- */

/**
 * Get the current value of a software timer
 * @param[in]  a_e8Id  Timer id
 * @return  uint16_t  current value if timer [0:0x8000] x CT_PERIODIC_RATE us
 */
STATIC INLINE uint16_t swtimer_getCurrent(swtimer_id_t a_e8Id);

/**
 * Get the period value of a software timer
 * @param[in]  a_e8Id  Timer id
 * @return  uint16_t  current period if timer [0:0x8000] x CT_PERIODIC_RATE us
 */
STATIC INLINE uint16_t swtimer_getPeriod(swtimer_id_t a_e8Id);

#ifndef UNITTEST
#include "swtimer_inline_impl.h"
#endif /* UNITTEST */

#endif  /* SWTIMER_H */

/* EOF */
