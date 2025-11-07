/**
 * @file
 * @brief The adc module definitions.
 * @internal
 *
 * @copyright (C) 2018-2020 Melexis N.V.
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
 * @details This file contains the definitions of the adc module.
 */

#ifndef ADC_H_
#define ADC_H_

#include <compiler_abstraction.h>
#include <stdbool.h>
#include <stdint.h>
#include <lib_adc.h>
#include "defines.h"

/* ---------------------------------------------
 * Public Enumerations
 * --------------------------------------------- */

/** adc sample index enum */
typedef enum
{
    // 6°³ 50us
    // io1,2,3,4
    // Cycle 1
    ADC_SAMPLE_VSM = 0, /**< sample index: motor supply voltage */
    ADC_SAMPLE_TEMP,    /**< sample index: chip temperature */
    ADC_SAMPLE_CURR,    /**< sample index: motor current */
    ADC_SAMPLE_VDDA,    /**< sample index: VDDA */
    ADC_SAMPLE_IGN,     /**< sample index: IGN */
    ADC_SAMPLE_VS,      /**< sample index: supply voltage */
                        // Cycle 2
    ADC_SAMPLE_GMR_IO1, /**< sample index: GMR -Cos */
    ADC_SAMPLE_GMR_IO2, /**< sample index: GMR -Sin */
    ADC_SAMPLE_CURR_2,  /**< sample index: motor current2 */
    ADC_SAMPLE_GMR_IO3, /**< sample index: GMR +Cos */
    ADC_SAMPLE_GMR_IO4, /**< sample index: GMR + Sin */
    ADC_SAMPLE_VS_2,    /**< sample index: supply voltage */
} adc_SBase_t;          /**< adc sample index type */

/**
 * @brief callback type
 */
typedef void (*adc_irq_t)(void);

extern uint16_t l_u16MotorSupplyVoltageFilter;
extern uint16_t l_u16SupplyVoltageFilter;
extern uint16_t l_u16ChipTemperatureFilter;
extern uint16_t g_u16IOADC1, g_u16IOADC2, g_u16IOADC3, g_u16IOADC4;

/* ---------------------------------------------
 * Public Function Declarations
 * --------------------------------------------- */

void adc_Init(void);
void adc_Close(void);
void adc_Start(bool bWait);
void adc_Stop(void);
void adc_Shunt_OffsetCalib(void);
uint16_t adc_CaptureOneChannel(AdcSignal_t channel);
int16_t adc_ConvertToTchip(uint16_t u16AdcVal);
int16_t adc_ConvertToVsmFiltered(uint16_t u16AdcVal);
int16_t adc_ConvertToVsupply(uint16_t u16AdcVal);
int16_t adc_ConvertToVphase(uint16_t u16AdcVal);
int16_t adc_ConvertToVio(uint16_t u16AdcVal);
int16_t adc_ConvertToVoltage(uint16_t u16AdcVal);
int16_t adc_ConvertToCurrent(uint16_t u16AdcVal);

/* ---------------------------------------------
 * Public Variables
 * --------------------------------------------- */

/** Get the raw Vsm
 *
 * @returns  raw Vsm adc sample.
 */
static INLINE uint16_t adc_GetRawVsm(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_VSM];
}

/** Get the raw Vs
 *
 * @returns  raw Vs adc sample.
 */
static INLINE uint16_t adc_GetRawVs(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_VS];
}

/** Get the raw temperature
 *
 * @returns  raw temperature adc sample.
 */
static INLINE uint16_t adc_GetRawTemperature(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_TEMP];
}

/** Get the raw VDDA voltage sample
 *
 * @returns  raw VDDA voltage sample.
 */
static INLINE uint16_t adc_GetRawVdda(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_VDDA];
}

/** Get the raw IO voltage sample
 *
 * @returns  raw IO voltage sample.
 */
static INLINE uint16_t adc_GetRawIGN(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_IGN];
}

/** getter for the motor current zero offset value
 *
 * @return  the measured motor current zero offset value in adc bits.
 * @remark  if the application did not call adc_Shunt_OffsetCalib()
 *          before calling this function the Melexis calibration data
 *          is returned.
 */
static INLINE int16_t adc_GetCurrentZeroOffset(void)
{
    extern int16_t i16MotorCurrentZeroOffset;
    return i16MotorCurrentZeroOffset;
}

/** Get the raw Current
 *
 * @returns  raw Current adc sample.
 */
static INLINE uint16_t adc_GetRawCurrent(void)
{
    extern volatile uint16_t dBase[];
    extern int16_t i16MotorCurrentZeroOffset;
    int16_t i16MotorCurrent = ((dBase[ADC_SAMPLE_CURR] + dBase[ADC_SAMPLE_CURR_2]) >> 1);
    if (i16MotorCurrent >= i16MotorCurrentZeroOffset)
    {
        i16MotorCurrent -= i16MotorCurrentZeroOffset;
    }
    else
    {
        i16MotorCurrent = 0;
    }
    return (i16MotorCurrent);
}

///////////////////
/** Get the raw IO voltage sample
 *
 * @returns  raw IO voltage sample.
 */
static INLINE uint16_t adc_Get_GMR_nCosine(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_GMR_IO1];
}

/** Get the raw IO voltage sample
 *
 * @returns  raw IO voltage sample.
 */
static INLINE uint16_t adc_Get_GMR_nSine(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_GMR_IO2];
}
/** Get the raw IO voltage sample
 *
 * @returns  raw IO voltage sample.
 */
static INLINE uint16_t adc_Get_GMR_pCosine(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_GMR_IO3];
}
/** Get the raw IO voltage sample
 *
 * @returns  raw IO voltage sample.
 */
static INLINE uint16_t adc_Get_GMR_pSine(void)
{
    extern volatile uint16_t dBase[];
    return dBase[ADC_SAMPLE_GMR_IO4];
}

////////////////////

/**
 * register a callback from ADC interrupt
 */
static INLINE void adc_RegisterIRQ(adc_irq_t value)
{
    extern adc_irq_t p16AdcIrq;
    p16AdcIrq = value;
}

#if 0 // defined(APP_HAS_DEBUG)
/**
 * register a callback from ADC interrupt
 */
static INLINE void adc_RegisterIRQ2(adc_irq_t value)
{
    extern adc_irq_t p16AdcIrq2;
    p16AdcIrq2 = value;
}
#endif

#endif /* ADC_H_ */

/* EOF */
