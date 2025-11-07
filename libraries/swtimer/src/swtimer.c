/**
 * @file
 * @brief The software timer library.
 * @internal
 *
 * @copyright (C) 2019-2020 Melexis N.V.
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
 * @details
 * This file contains the implementations of the software timer library.
 */

#include <syslib.h>
#include <string.h>
#include <itc_helper.h>
#include <mathlib.h>
#include <timerlib.h>
#include <atomic.h>
#include <stdint.h>
#include <lib_clock.h>
#include <conv_clock_error.h>
//#include "fw_ints_prio.h"
#include "swtimer_config.h"
#include "swtimer.h"


//#include <lib_softio.h>

//250414 - PWM Dual Capture
//#include <pwm_capture.h>
//#include <pwm_capture_config.h>
//#include <pwm_communication.h>
//#include <pwm_communication_config.h>

/* ---------------------------------------------
 * Configuration Check
 * --------------------------------------------- */

/*
 * A 14 bits free running counter clocked by 1MHz is available to generate TIMER_IT
 * interrupt at a rate varying from 1us (TIMER [13:0] = 1) to 16383us (TIMER [13:0] = 16383).
 */
#if CT_PERIODIC_RATE < 1
#error "CT_PERIODIC_RATE value is out of limit"
#endif
#if CT_PERIODIC_RATE > 16383
#error "CT_PERIODIC_RATE value is out of limit"
#endif


/* ---------------------------------------------
 * Variables
 * --------------------------------------------- */

swtimer_config_t g_pTimer[SWTIMER_NR_OF];  /**< Array of timers */


/* ---------------------------------------------
 * Public Function Implementations
 * --------------------------------------------- */

/**
 * Initialize Core Timer and start.
 */
void swtimer_init(void)
{
    /* Init clock error conversion */
    conv_clock_error_init();

    /* Init Timer in 1us mode */
    STIMER_INIT(STIMER_1US_CLOCK, CT_PERIODIC_RATE);

    /* Enable Timer interrupt */
    #ifndef UNITTEST
    ENTER_SECTION(SYSTEM_MODE);
    Itc_Clear(STIMER);
    Itc_Enable(STIMER);
 //   Itc_SetPrio(STIMER, PRIO_STIMER_INT);

    EXIT_SECTION();
    #endif

    /* Clear all */
    (void)memset((void *)g_pTimer, 0, sizeof(g_pTimer) / sizeof(uint8_t));
}

/**
 * Disable Simple/Core Timer
 */
void swtimer_deinit(void)
{
    STIMER_SET_MODE(STIMER_DISABLE_CLOCK);  /* switch off */

    /* Disable Timer interrupt */
    #ifndef UNITTEST
    ENTER_SECTION(SYSTEM_MODE);
    Itc_Disable(STIMER);
    EXIT_SECTION();
    #endif
}


/**
 * swtimer period calibration over temperature
 *
 * Next figure shows the core timer frequency, calibrated every 10C gradient
 *
 * \image html swtimer_calibrate.png
 * \image latex swtimer_calibrate.png
 *
 * @param[in]  u16ADC_Value  IC temperature raw ADC measurement.
 */
void swtimer_calibrate(uint16_t u16ADC_Value)
{
    int16_t i16ClockCountCorrection = 0;

    /* Get clock count correction */
    i16ClockCountCorrection = conv_clock_error_speed(u16ADC_Value);

    i16ClockCountCorrection = conv_clock_error_correct_period(CT_PERIODIC_RATE, i16ClockCountCorrection);
    STIMER_SET_VALUE(i16ClockCountCorrection);
}

/**
 * Register a new sw timer
 * @param  a_e8Id  the index
 * @param  a_u15Load  a 15-bit load value [0:0x8000] x CT_PERIODIC_RATE us
 * @param  a_eMode  timer mode, single shot or repetitive
 * @return  true  in case of success
 */
bool swtimer_register(swtimer_id_t a_e8Id, uint16_t a_u15Load, swtimer_mode_t a_eMode)
{
    bool bRetVal = false;

    if ((a_e8Id < SWTIMER_NR_OF) && (a_u15Load < 0x8000))
    {
        g_pTimer[a_e8Id].bRepetitive = (a_eMode == REPETITIVE) ? 1u : 0u;
        g_pTimer[a_e8Id].u15Load = a_u15Load;
        bRetVal = true;
    }
    return bRetVal;
}

/**
 * Start a software timer
 * @param[in]  a_e8Id  Timer id
 */
void swtimer_start(swtimer_id_t a_e8Id)
{
    g_pTimer[a_e8Id].u15Current = g_pTimer[a_e8Id].u15Load;
    g_pTimer[a_e8Id].bTriggered = 0u;
}

/**
 * Stop a software timer.
 * @param[in]  a_e8Id  Timer id
 */
void swtimer_stop(swtimer_id_t a_e8Id)
{
    g_pTimer[a_e8Id].u15Current = 0u;
    g_pTimer[a_e8Id].bTriggered = 0u;
}

/**
 * Check if a software timer is stopped by timer id, meaning not running, trigger cleared.
 *
 * @warning  should only be called from lower interrupt priority
 *           level as SWTIMER_INT
 *
 * @param[in]  a_e8Id  Timer id
 * @return  true  if stopped
 */
bool swtimer_isStopped(swtimer_id_t a_e8Id)
{
    return ((g_pTimer[a_e8Id].bTriggered == 0u) && (g_pTimer[a_e8Id].u15Current == 0u)) ? true : false;
}

/**
 * Check if a software timer is running by timer id.
 * @param[in]  a_e8Id  Timer id
 * @return  true  if running
 */
bool swtimer_isRunning(swtimer_id_t a_e8Id)
{
    return ((g_pTimer[a_e8Id].u15Current != 0u) ? true : false);
}

/**
 * Check if a software timer has expired and clear it by timer id.
 * @param[in]  a_e8Id  Timer id
 * @return  true  if expired
 */
bool swtimer_isTriggered(swtimer_id_t a_e8Id)
{
    bool lb_trigged = false;

    if (g_pTimer[a_e8Id].bTriggered != 0u)
    {
        g_pTimer[a_e8Id].bTriggered = 0u;
        lb_trigged = true;
    }

    return (lb_trigged);
}

/**
 * The core timer interrupt service routine.
 */
INTERRUPT void _STIMER_INT(void)
{
    swtimer_enterIrq();

    /* Count-down all the registered SW timers, trigger when 0 is reached */
    for (uint16_t index = 0u; index < (uint16_t)SWTIMER_NR_OF; index++)
    {
        uint16_t value = g_pTimer[index].u15Current;

        if (value != 0u)
        {
            value--;

            if (value == 0u)
            {
                /* Trigger state */
                g_pTimer[index].bTriggered = 1u;

                /* Callback event */
                swtimer_triggerIrq(index);

                if (g_pTimer[index].bRepetitive != 0u)
                {
                    /* Reload periodic timers */
                    value = g_pTimer[index].u15Load;
                }
            }

            g_pTimer[index].u15Current = value;
        }
    }

    swtimer_exitIrq();
}


/* ---------------------------------------------
 * Weak, default function implementation
 * --------------------------------------------- */

WEAK void swtimer_enterIrq(void)
{}

WEAK void swtimer_exitIrq(void)
{}

WEAK void swtimer_triggerIrq(uint16_t id)
{
    (void)id;
}

#ifdef UNITTEST
#include "swtimer_inline_impl.h"
#endif

/* EOF */
