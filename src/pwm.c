/**
 * @file
 * @brief The pwm driver module.
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
 * @details This file contains the implementations of the pwm driver module.
 */
#include <sys_tools.h>
#include <io.h>
#include <plib.h>
#include <mathlib.h>
#include <fm.h>
#include <lib_pwm.h>
#include <lib_motor_driver.h>
#include <lib_softio.h>

#include "pwm.h"
#include "defines.h"

/* ---------------------------------------------
 * Local Constants
 * --------------------------------------------- */

/* ---------------------------------------------
 * Local Defines
 * --------------------------------------------- */
/* Fcnt = (Fpll / (M + 1)) * (1 / (2^N))
 * Fpwm = (Fcnt / (PPER + 1))
 * PPER + 1 = (Fcnt / Fpwm)
 */

/** pwm signal frequency in Hz */
#define PWM_FREQ 20000.0
/** pwm counter prescaler M value */
#define PWM_PSCL_M 0u
/** pwm counter prescaler N value */
#define PWM_PSCL_N 0u
/** PWM clock */
#define PWM_FCNT ((((float)FPLL * 1000.0) / (PWM_PSCL_M + 1.0)) * (1.0 / (1 << PWM_PSCL_N)))
/** PWM_PERIOD is in range 0..32767 (15-bit only as we use shift in pwm_SetDutyCycle() */
#define PWM_PERIOD (uint16_t)((PWM_FCNT / PWM_FREQ) - 1)

/**
 * Small correction delay to synchronize PWM and ADC measurements
 */
#define C_PWM_DCORR 0

/** 1th ADC sample point = 17% PWM period	MA1 */
#define PWM_CMP_1 ((uint16_t)(((1u * PWM_PERIOD) + 3u) / 6u) + C_PWM_DCORR)
/** 2th ADC sample point = 33% PWM period	SL1 */
#define PWM_CMP_2 ((uint16_t)(((2u * PWM_PERIOD) + 3u) / 6u) + C_PWM_DCORR)
/** 3th ADC sample point = 50% PWM period	SL2 */
#define PWM_CMP_3 ((uint16_t)(((3u * PWM_PERIOD) + 3u) / 6u) + C_PWM_DCORR)
/** 4th ADC sample point = 67% PWM period	SL3 */
#define PWM_CMP_4 ((uint16_t)(((4u * PWM_PERIOD) + 3u) / 6u) + C_PWM_DCORR)
///** 5th ADC sample point = 83% PWM period SL4 */
#define PWM_CMP_5 ((uint16_t)(((5u * PWM_PERIOD) + 3u) / 6u) + C_PWM_DCORR)

/* ---------------------------------------------
 * Local Enumerations
 * --------------------------------------------- */

/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */
static uint16_t u16DutyCycleMax;     /**< the maximum pwm duty cycle */
static uint16_t u16LastDiagErr = 0u; /**< last diagnostic error code */
static uint16_t u16LTcopy[4];        /**< LT registers values to be written during Master1 End ISR */

uint16_t pwm_u16DutyCycle = 0u;                                  /**< [0:C_CORRECTION_RATIO_MAX] PWM duty cycle */
uint16_t g_u16HalfPwmMin = (uint16_t)((0.05f * PWM_PERIOD) / 2); /**< [0:PWM_PERIOD/2] min pwm period */
uint16_t g_u16HalfPwmMax = (uint16_t)((0.95f * PWM_PERIOD) / 2); /**< [0:PWM_PERIOD/2] max pwm period */

uint8_t a, b, c, d;
uint16_t aa, bb, cc, dd;
/* ---------------------------------------------
 * Configuration Check
 * --------------------------------------------- */

/* ---------------------------------------------
 * Local Function Declarations
 * --------------------------------------------- */

/** Initialize the pwm driver module
 *
 * This function initializes the pwm module and configures
 * the driver module.
 *
 * PWM_POL_HIGH
 * LT - 0%   : MIN PWMDC
 * LT - 100% : MAX PWMDC
 *
 *     ----->|<-------PERIOD------>|
 *     _____________         ________
 *    |      |      |_______|      |
 *     <-LT->|<-LT->|        <-LT->|
 *
 *
 *  * PWM_POL_LOW
 * LT - 0%   : MAX PWMDC
 * LT - 100% : MIN PWMDC
 *     ----->|<-------PERIOD------>|
 *                   _______
 *    |______|______|       |______|_
 *     <-LT->|<-LT->|        <-LT->|
 *
 */
void pwm_Init(void)
{

    pwm1_master_disable(); /* disable master1 */
    pwm1_disable();        /* disable slave 1 */
    pwm2_disable();        /* disable slave 2 */
    pwm3_disable();        /* disable slave 3 */
    pwm2_master_disable(); /* disable master2 */

    pwm1_master_init(
        PWM_PSCL_M + 1u, /* DIV */
        PWM_PSCL_N,      /* DIVN */
        PWM_PERIOD + 1u, /* PERIOD */
        0u,              /* LT */
        0u,              /* HT */
        PWM_CMP_1,       /* CMP */
        PWM_MODE_MIRROR, /* MODE */
#if (PWM_POL_METHOD == uPWM_POL_LOW)
        PWM_POL_LOW, /* POL */
#elif (PWM_POL_METHOD == uPWM_POL_HIGH)
        PWM_POL_HIGH, /* POL */
#endif
        0u /* InvertPolarityIDLE */
    );

    pwm1_init(
        PWM_PSCL_M + 1u, /* DIV */
        PWM_PSCL_N,      /* DIVN */
        0u,              /* LT */
        0u,              /* HT */
        PWM_CMP_2,       /* CMP */
        PWM_MODE_MIRROR, /* MODE */
#if (PWM_POL_METHOD == uPWM_POL_LOW)
        PWM_POL_LOW, /* POL */
#elif (PWM_POL_METHOD == uPWM_POL_HIGH)
        PWM_POL_HIGH, /* POL */
#endif
        0u /* InvertPolarityIDLE */
    );

    pwm2_init(
        PWM_PSCL_M + 1u, /* DIV */
        PWM_PSCL_N,      /* DIVN */
        0u,              /* LT */
        0u,              /* HT */
        PWM_CMP_3,       /* CMP */
        PWM_MODE_MIRROR, /* MODE */
#if (PWM_POL_METHOD == uPWM_POL_LOW)
        PWM_POL_LOW, /* POL */
#elif (PWM_POL_METHOD == uPWM_POL_HIGH)
        PWM_POL_HIGH, /* POL */
#endif
        0u /* InvertPolarityIDLE */
    );

    pwm3_init(
        PWM_PSCL_M + 1u, /* DIV */
        PWM_PSCL_N,      /* DIVN */
        0u,              /* LT */
        0u,              /* HT */
        PWM_CMP_4,       /* CMP */
        PWM_MODE_MIRROR, /* MODE */
#if (PWM_POL_METHOD == uPWM_POL_LOW)
        PWM_POL_LOW, /* POL */
#elif (PWM_POL_METHOD == uPWM_POL_HIGH)
        PWM_POL_HIGH, /* POL */
#endif
        0u /* InvertPolarityIDLE */
    );

    pwm2_master_init(
        PWM_PSCL_M + 1u, /* DIV */
        PWM_PSCL_N,      /* DIVN */
        PWM_PERIOD + 1u, /* PERIOD */
        0u,              /* LT */
        0u,              /* HT */
        PWM_CMP_5,       /* CMP */
        PWM_MODE_MIRROR, /* MODE */
#if (PWM_POL_METHOD == uPWM_POL_LOW)
        PWM_POL_LOW, /* POL */
#elif (PWM_POL_METHOD == uPWM_POL_HIGH)
        PWM_POL_HIGH, /* POL */
#endif
        0u /* InvertPolarityIDLE */
    );

    pwm1_master_enable(); /* enable master1 */
    pwm1_enable();        /* enable slave 1 */
    pwm2_enable();        /* enable slave 2 */
    pwm3_enable();        /* enable slave 3 */
    pwm2_master_enable(); /* enable master2 */

    /* all phases in tri-state */
    //     drvcfg_Tristate(); in motor3ph_pwm_private.h
    MotorDriverUVWTSelectSource(DRV_CTRL_TRISTATE,  /* U */
                                DRV_CTRL_TRISTATE,  /* V */
                                DRV_CTRL_TRISTATE,  /* W */
                                DRV_CTRL_TRISTATE); /* T */

    /* enable digital part of the driver */
    // drvcfg_Enable(); in motor3ph_pwm_private.h
    IO_SET(PORT_DRV2_PROT, DIS_DRV, 0u);

    IO_SET(PORT_DRV_OUT, ENABLE_DRVSUP, 0x1u); /* enable driver supply */

    /* wait some time to have Vddaf to stabilize */
    DELAY_US(10u);

    /* enable the driver */
    IO_SET(PORT_DRV_OUT,
           ENABLE_DRVMOD_CPCLK, 0x1u, /* enable driver clock */
           ENABLE_DRV, 0x0u,          /* enable drivers of phases U,V,W,T */
           ENABLE_CSA, 0x1u,          /* enable current sense amplifier */
           ENABLE_HS_OC, 0x1u,        /* enable high-side FET VDS over-voltage / over-current detection */
           ENABLE_LS_OC, 0x1u,        /* enable low-side FET VDS over-voltage / over-current detection */
           DRVMOD_OPTION, 0x0u,
           PARALLEL_MODE_DRV, 0x1u); /* enable parallel mode U+V, W+T driver */

    /* configure pwm update sync interrupt - CNT ISR */
    ENTER_SECTION(ATOMIC_SYSTEM_MODE);
    Itc_SetPrio(PWM_MASTER1_END, 3u);
    pwm1_master_cnti_enable();
    pwm2_cmpi_enable(); // pwm2
    EXIT_SECTION();

    u16DutyCycleMax = 0x4000;
}

/** Start the pwm driver module
 *
 * This function will start the pwm driver and will enable the
 * output of the pwm signals on the driver pins.
 * @param[in]  u16DutyCycle  The new pwm duty cycle (u16DutyCycle < u16DutyCycleMax).
 */
void pwm_Start(uint8_t dir, uint16_t u16DutyCycle)
{
    /* errors are gone */
    u16LastDiagErr = 0u;

    //        if (u8Status == 0u)
    {
#if 0        
            /* clear UV_VS_MEM flag */
            IO_SET(PORT_DRV1_PROT, DIS_UV_VS, 1u);
            IO_SET(PORT_DRV1_PROT, DIS_UV_VS, 0u);

            /* clear OV_VS_MEM flag */
            IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 1u);
            IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 0u);

            /* (Re-)enable interrupts if they where disabled in interrupt handler */
            ENTER_SECTION(ATOMIC_SYSTEM_MODE);
            Itc_Clear(DIAG);
            Itc_Enable(DIAG);
            Itc_Clear(UV_VS);
            Itc_Enable(UV_VS);
            Itc_Clear(OV_VS);
            Itc_Enable(OV_VS);
			Itc_Clear(OVC);
			Itc_Enable(OVC);
			Itc_Clear(OVT);
			Itc_Enable(OVT);
			EXIT_SECTION();
#endif
        pwm_SetDutyCycle(dir, u16DutyCycle);
#if 0
            /* connect drivers to pwm blocks */
            MotorDriverUVWTSelectSource(DRV_CTRL_PWM_MASTER1,  /* U - Phase A */
            							DRV_CTRL_PWM_MASTER1,  /* V - Phase A */
										DRV_CTRL_PWM_SLAVE1,   /* W - Phase B */
										DRV_CTRL_PWM_SLAVE1);  /* T - Phase B */
#endif
        IO_SET(PORT_DRV_OUT, ENABLE_DRV, 0xFu); /* enable drivers of phases U,V,W,T */
    }
}

/** Update the pwm duty cycle for a channel
 *
 * This function will update the pwm duty cycle of a specific or
 * all channels with the value provided. The value will be clipped
 * to the maximum value set with pwm_SetMaxDutyCycle().
 * @param[in]  u8Channels    The channel(s) to update the duty cycle for.
 * @param[in]  u16DutyCycle  The new pwm duty cycle (u16DutyCycle < u16DutyCycleMax).
 */
extern uint16_t g_u16debug_2, g_u16debug_5;
void pwm_SetDutyCycle(uint8_t dir, uint16_t u16DutyCycle)
{
    uint16_t u16LT;

    if (u16DutyCycle > C_PWMOUT_MAX_DUTY)
    {
        u16DutyCycle = C_PWMOUT_MAX_DUTY;
    }
    u16DutyCycle = (C_PWMOUT_MAX_DUTY - u16DutyCycle);
    // u16DutyCycle = u16DutyCycle<<2;

    /* u16DutyCycle < 0x4000
     * in PWM_MIRROR_MODE : 0 < u16LT < PWM_PERIOD / 2
     * u16LT = ( (u16DutyCycle << 2) * PWM_PERIOD/2 ) / (1<<16) = % * PWM_PERIOD/2
     *
     * |<---------PER--------->|
     * |    |<-LT->|<-LT->|    |
     * |                       |
     * |     _____________     |
     * |____|             |____|
     */

    /* Change direction */
    switch (dir)
    {
    case C_DIR_CW:
        // UV : PWM, WT : GND
        /* connect drivers to pwm blocks */
        MotorDriverUVWTSelectSource(DRV_CTRL_PWM_MASTER1, /* U - Phase A */
                                    DRV_CTRL_PWM_MASTER1, /* V - Phase A */
                                    DRV_CTRL_LOW,         /* W - Phase B */
                                    DRV_CTRL_LOW);        /* T - Phase B */
        break;
    case C_DIR_CCW:
        // UV : GND, WT, PWM
        /* connect drivers to pwm blocks */
        MotorDriverUVWTSelectSource(DRV_CTRL_LOW,         /* U - Phase A */
                                    DRV_CTRL_LOW,         /* V - Phase A */
                                    DRV_CTRL_PWM_SLAVE3,  /* W - Phase B */
                                    DRV_CTRL_PWM_SLAVE3); /* T - Phase B */
        break;
    default:
        break;
    } /* Change direction */

    u16LT = mulU16hi_U16byU16(u16DutyCycle, PWM_PERIOD << 4);

    u16LTcopy[0] = u16LT; /* U */
    u16LTcopy[1] = u16LT; /* V */
    u16LTcopy[2] = u16LT; /* W */
    u16LTcopy[3] = u16LT; /* T */
}

/** Set the maximum allowed pwm duty cycle
 *
 * Configure the maximum allowed pwm duty cycle. Default is
 * 0x4000.
 * @param[in]  u16DutyCycle  The new maximum pwm duty cycle
 */
void pwm_SetMaxDutyCycle(uint16_t u16DutyCycle)
{
    if ((u16DutyCycle > 0u) && (u16DutyCycle <= 0x4000u))
    {
        u16DutyCycleMax = u16DutyCycle;
    }
}

/** Stop the pwm driver module
 *
 * Stop the pwm driver module and disable the motor driver output
 * pins from outputting the pwn signals.
 */
void pwm_Stop(void)
{
    /* all phases connected to GND */
    MotorDriverUVWTSelectSource(DRV_CTRL_LOW,  /* U */
                                DRV_CTRL_LOW,  /* V */
                                DRV_CTRL_LOW,  /* W */
                                DRV_CTRL_LOW); /* T */

    /* stop pwm modules output */
    IO_SET(PWM_MASTER1, LT, 0u); /* U */
    IO_SET(PWM_SLAVE1, LT, 0u);  /* V */
    IO_SET(PWM_SLAVE2, LT, 0u);  /* W */
    IO_SET(PWM_SLAVE3, LT, 0u);  /* T */
}
void pwm_Off(void)
{
    /* all phases connected to GND */
    MotorDriverUVWTSelectSource(DRV_CTRL_TRISTATE,  /* U */
                                DRV_CTRL_TRISTATE,  /* V */
                                DRV_CTRL_TRISTATE,  /* W */
                                DRV_CTRL_TRISTATE); /* T */

    /* stop pwm modules output */
    IO_SET(PWM_MASTER1, LT, 0u); /* U */
    IO_SET(PWM_SLAVE1, LT, 0u);  /* V */
    IO_SET(PWM_SLAVE2, LT, 0u);  /* W */
    IO_SET(PWM_SLAVE3, LT, 0u);  /* T */
}
/** Disable the pwm driver module
 *
 * Puts the output phases in tri-state and disables the pwm
 * modules.
 */
void pwm_Disable(void)
{
    /* all phases in tristate */
    MotorDriverUVWTSelectSource(DRV_CTRL_TRISTATE,  /* U */
                                DRV_CTRL_TRISTATE,  /* V */
                                DRV_CTRL_TRISTATE,  /* W */
                                DRV_CTRL_TRISTATE); /* T */

    /* stop pwm modules output */
    IO_SET(PWM_MASTER1, LT, 0u); /* U */
    IO_SET(PWM_SLAVE1, LT, 0u);  /* V */
    IO_SET(PWM_SLAVE2, LT, 0u);  /* W */
    IO_SET(PWM_SLAVE3, LT, 0u);  /* T */

    IO_SET(PORT_DRV_OUT,
           ENABLE_DRV, 0x0u,           /* disable drivers of all phases */
           ENABLE_DRVSUP, 0x0u,        /* disable driver supply */
           ENABLE_DRVMOD_CPCLK, 0x0u); /* disable driver clock */

    IO_SET(PORT_DRV2_PROT, DIS_DRV, 1u); /* disable digital part of the driver */

    ENTER_SECTION(ATOMIC_SYSTEM_MODE);
    Itc_Disable(DIAG);
    Itc_Disable(UV_VS);
    Itc_Disable(OV_VS);
    EXIT_SECTION();
}

/** Interrupt handler for PWM_MASTER1_END.
 *
 * This function will handle the end of period interrupt to produce a
 * synchronized PWM LT update.
 */
INTERRUPT void _PWM_MASTER1_END_INT(void)
{
    IO_SET(PWM_MASTER1, LT, u16LTcopy[0]); /* U */
    IO_SET(PWM_SLAVE1, LT, u16LTcopy[1]);  /* V */
    IO_SET(PWM_SLAVE2, LT, u16LTcopy[2]);  /* W */
    IO_SET(PWM_SLAVE3, LT, u16LTcopy[3]);  /* T */

    //    ENTER_SECTION(ATOMIC_SYSTEM_MODE);
    //    Itc_Disable(PWM_MASTER1_END);
    //    EXIT_SECTION();
}

INTERRUPT void _PWM_SLAVE2_CMP_INT(void)
{
}

/** Generic motor driver error interrupt handler
 *
 * This function will handle the generic motor driver
 * interrupt.
 */
INTERRUPT void _DIAG_INT(void)
{
    /* remember the diagnostic error, all diagnostic errors need to be
     * cleared before the driver can be started again
     */
    u16LastDiagErr = IO_HOST(PORT_DIAG_IN, OVT_MEM);

    pwm_Stop();

    /* disable the interrupt, otherwise the interrupt will be called continuously */
    Itc_Disable(DIAG);
}

/* EOF */
