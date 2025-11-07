/**
 * @file
 * @brief The adc module.
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
 * @details This file contains the implementations of the adc module.
 */
#include <io.h>
#include <itc_helper.h>
#include <lib_adc.h>
#include <lib_calculate_temperature.h>
#include <mathlib.h>
#include <conv_high_voltage.h>
#include <conv_low_voltage.h>
#include <conv_motor_voltage.h>
#include <conv_shunt_current.h>
#include <eeprom_map.h>
#include "fw_ints_prio.h"
#include "adc.h"
#include "defines.h"

/* ---------------------------------------------
 * Local Defines
 * --------------------------------------------- */

/** adc settling time in us */
#define C_ADC_SETTLING_TIME 5U

/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */
uint16_t l_u16MotorSupplyVoltageFilter = 0u;
uint16_t l_u16SupplyVoltageFilter = 0u;
uint16_t l_u16ChipTemperatureFilter = 0u;
uint16_t g_u16IOADC1, g_u16IOADC2, g_u16IOADC3, g_u16IOADC4;

extern volatile uint16_t dBase[];
/** adc measurement configuration table @warning this table must be located in RAM */
AdcSData_t sBase[] =
    {
        {.u16 = (uint16_t)&dBase[0]},
        // Cycle 1st of ADC
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_VSM_DIV21,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA1_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 17%  Vsm */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_TEMP,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_SL1_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 33%  Temp */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_CSOUT,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_SL2_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 50%  Motor current */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_VDDA_DIV2, /* 3.3V / 2 = 1.75(1024adc) */
          .adcRef = ADC_VREF_MEAS,
          .adcTrig = ADC_TRIG_PWM_SL3_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 67%  VDDA */
#if VALVE_IGN_PIN == 0
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_IO0_DIV1_36, // sturation at 3.676V
          .adcRef = ADC_VREF_MEAS,        // ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA2_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 83%  IO3 - feedback of VDDA(3.3V) */
#else
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_IO5_DIV1_36, // sturation at 3.676V
          .adcRef = ADC_VREF_MEAS,        // ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA2_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 83%  IO3 - feedback of VDDA(3.3V) */
#endif
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_VS_DIV21,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA1_END,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 100% VS */

        // Cycle 2nd of ADC
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_IO1_DIV1_36,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA1_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 17%  IO1 */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_IO2_DIV1_36,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_SL1_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 33%  IO2 */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_CSOUT,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_SL2_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 50%  Motor current */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_IO3_DIV1_36, /*  */
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_SL3_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 67%   */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_IO4_DIV1_36,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA2_CMP,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 83%   */
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = ADC_SIG_VS_DIV21,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA1_END,
          .adcDiv = ADC_DIV_DISABLED}}, /**< 100% VS */
        {{.adcEosSign = ADC_EOF_SIGN}}, /**< end marker */
        {.u16 = (uint16_t)&sBase[0]}};
volatile uint16_t dBase[(sizeof(sBase) / sizeof(uint16_t)) - 2]; /**< adc measurement buffer (adc measurements + crc) */
int16_t i16MotorCurrentZeroOffset = 0;                           /**< shunt offset voltage */
adc_irq_t p16AdcIrq = NULL;                                      /**< adc irq callback */
#if defined(APP_HAS_DEBUG)
adc_irq_t p16AdcIrq2 = NULL; /**< adc irq callback */
#endif

/* ---------------------------------------------
 * Local Functions
 * --------------------------------------------- */

static bool adc_DoSoftwareTrigger(AdcPhaseState_t NextState);

/* ---------------------------------------------
 * Public Functions Implementation
 * --------------------------------------------- */

/**
 * Initialize the adc module
 */
void adc_Init(void)
{
    conv_high_voltage_init();
    conv_low_voltage_init();
    conv_motor_voltage_init();
    conv_shunt_current_init();
    conv_update_shunt_current_offset(0);

    /* enable VSM low-pass filter */
    IO_SET(PORT_MISC2_OUT, VSM_FILT_ON, 1u);
    /* wait for about 1.25ms to settle; measurements of the VSMF channel before the end of the delay are incorrect */
    DELAY_US(1250);

    /* enable current sense amplifier */
    IO_SET(PORT_DRV_OUT, ENABLE_CSA, 1u);

    /* enable the adc module */
    IO_SET(PORT_ADC_CTRL, ADC_EN, 1u);

    /* enable adc interrupts */
    ENTER_SECTION(ATOMIC_SYSTEM_MODE);
    Itc_SetPrio(ADC_SAR, PRIO_ADC_SAR_INT);
    Itc_Enable(ADC_SAR);
    EXIT_SECTION();
}

/**
 * Disable the adc module
 */
void adc_Close(void)
{
    /* disable interrupt */
    Itc_Disable(ADC_SAR);

    /* disable the adc module */
    IO_SET(PORT_ADC_CTRL, ADC_EN, 0u);
}

/** Initialize and start the adc module
 *
 * This function initialize the adc module and clears/disables it's
 * corresponding interrupt.
 * @param[in]  bWait  true : wait for 1 conversion cycle to be finished.
 */
void adc_Start(bool bWait)
{
    /* prior to starting the adc clear all available errors */
    AdcClearAllErrors();
    AdcResume();

    /* create adc init structure */
    AdcControl_t adc_ctrl = {{.sosSource = ADC_SOS_FIRST_HARDWARE_TRIGGER,
                              .socSource = ADC_SOC_HARDWARE_TRIGGER,
                              .noInterleave = 1u,
                              .saturate = 1u,
                              .intScheme = ADC_INT_EOF,
                              .asb = ADC_ASB_NEVER,
                              .adcWidth = ADC_WDT_16Bit}};

    /* initialize adc unit */
    AdcInit(((FPLL / 4000u) - 1u), (void *)&sBase[0], adc_ctrl);

    /* start the adc sequence */
    ENTER_SECTION(ATOMIC_SYSTEM_MODE);
    Itc_Clear(ADC_SAR);
    EXIT_SECTION();
    AdcStart();
    if (bWait)
    {
        /* wait to fill buffers with valid data */
        while (Itc_IsPending(ADC_SAR) == 0u)
        {
        }
    }
}

/** Stop the adc module
 *
 * This function tries to stop the adc using the recommended method, it will
 * wait for some time to wait till the adc really stopped.
 */
void adc_Stop(void)
{
    if (IO_GET(ADC_SAR, STOP) != 1u)
    {
        /* Stop ADC */
        while (IO_GET(ADC_SAR, STOP) == 0u)
        {
            AdcStop();
        }

        while (IO_GET(ADC_SAR, STATE) != ADC_PHASE_IDLE)
        {
            /* Restart ADC ...*/
            while (IO_GET(ADC_SAR, START) == 0u)
            {
                AdcStart();
            }

            NOP();

            /* ... and STOP it again */
            while (IO_GET(ADC_SAR, STOP) == 0u)
            {
                AdcStop();
            }
        }
    }
}

/** Do the current shunt offset calibration measurement
 *
 * This function will measure the shunt offset value for later use in
 * calculations by the application.
 *
 * @remark  Driver must be enabled before calling this function!
 */
void adc_Shunt_OffsetCalib(void)
{
    volatile uint16_t ZCO[3]; /**< adc sample buffer (2 samples + 1 crc) */
    volatile AdcSData_t sBase_CurrOff[] =
        {
            {.u16 = (uint16_t)&ZCO[0]},
            {{.adcEosSign = ADC_NO_SIGN,
              .adcChan = ADC_SIG_CSOUT,
              .adcRef = ADC_VREF_2_5V,
              .adcTrig = ADC_TRIG_PWM_MA1_CMP,
              .adcDiv = ADC_DIV_DISABLED}},
            {{.adcEosSign = ADC_NO_SIGN,
              .adcChan = ADC_SIG_CSOUT,
              .adcRef = ADC_VREF_2_5V,
              .adcTrig = ADC_TRIG_PWM_MA1_CMP,
              .adcDiv = ADC_DIV_DISABLED}},
            {{.adcEosSign = ADC_EOS_SIGN}},
        };

    //////// add below

    /* configure the adc for offset measurements */
    adc_Stop(); /* properly stop the adc */

    /* enable current sense amplifier */
    IO_SET(PORT_DRV_OUT, ENABLE_CSA, 1u);

    /* enable the adc module */
    IO_SET(PORT_ADC_CTRL, ADC_EN, 1u);

    /////////////////////
    /* create adc init structure */
    AdcControl_t adc_ctrl = {{.sosSource = ADC_SOS_FIRMWARE_TRIGGER,
                              .socSource = ADC_SOC_FIRMWARE_TRIGGER,
                              .noInterleave = 1u,
                              .saturate = 1u,
                              .intScheme = ADC_INT_NO_INT,
                              .asb = ADC_ASB_NEVER,
                              .adcWidth = ADC_WDT_16Bit}};

    /* initialize adc unit */
    AdcInit(((FPLL / 4000u) - 1u), (void *)&sBase_CurrOff[0], adc_ctrl);

    (void)adc_DoSoftwareTrigger(ADC_PHASE_WAIT_TRIGGER);
    (void)adc_DoSoftwareTrigger(ADC_PHASE_IDLE);

    i16MotorCurrentZeroOffset = (ZCO[0] + ZCO[1] + 1) / 2;

#if defined(MLX81330A01) || defined(MLX81330B01) || defined(MLX81332A01)
    int16_t diff = i16MotorCurrentZeroOffset - (512 + (int16_t)((int8_t)EE_GET(CURR_OFFS)));
#else /* MLX81330A01 || MLX81330B01 || MLX81332A01 */
    int16_t diff = i16MotorCurrentZeroOffset - (512 + (int16_t)((int8_t)EE_GET(O_CURR)));
#endif
    if ((diff < -10) || (diff > 10))
    {
        /* calibration data and measured data are diverging to much, there might be some electric error */
    }
}

uint16_t adc_CaptureOneChannel(AdcSignal_t channel)
{
    uint16_t retval;
    volatile uint16_t Temp[3]; /**< adc sample buffer (4 samples + 1 crc) */
    volatile AdcSData_t sBase_Temp[] =
        {
            {.u16 = (uint16_t)&Temp[0]},
            {{.adcEosSign = ADC_NO_SIGN,
              .adcChan = channel,
              .adcRef = ADC_VREF_2_5V,
              .adcTrig = ADC_TRIG_PWM_MA1_CMP, /* dummy */
              .adcDiv = ADC_DIV_DISABLED}},
            {{.adcEosSign = ADC_NO_SIGN,
              .adcChan = channel,
              .adcRef = ADC_VREF_2_5V,
              .adcTrig = ADC_TRIG_PWM_MA1_CMP, /* dummy */
              .adcDiv = ADC_DIV_DISABLED}},
#if 0          
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = channel,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA1_CMP,  /* dummy */
          .adcDiv = ADC_DIV_DISABLED}},
        {{.adcEosSign = ADC_NO_SIGN,
          .adcChan = channel,
          .adcRef = ADC_VREF_2_5V,
          .adcTrig = ADC_TRIG_PWM_MA1_CMP,  /* dummy */
          .adcDiv = ADC_DIV_DISABLED}},
        {{.adcEosSign = ADC_EOS_SIGN}},
#endif
        };

    /* configure the adc for sample measurements */
    adc_Stop(); /* properly stop the adc */

    /* create adc init structure */
    AdcControl_t adc_ctrl = {{.sosSource = ADC_SOS_FIRMWARE_TRIGGER,
                              .socSource = ADC_SOC_FIRMWARE_TRIGGER,
                              .noInterleave = 1u,
                              .saturate = 1u,
                              .intScheme = ADC_INT_NO_INT,
                              .asb = ADC_ASB_NEVER,
                              .adcWidth = ADC_WDT_16Bit}};

    /* initialize adc unit */
    AdcInit(((FPLL / 4000u) - 1u), (void *)&sBase_Temp[0], adc_ctrl);
#if 0
    (void)adc_DoSoftwareTrigger(ADC_PHASE_WAIT_TRIGGER);
    (void)adc_DoSoftwareTrigger(ADC_PHASE_WAIT_TRIGGER);
#endif
    (void)adc_DoSoftwareTrigger(ADC_PHASE_WAIT_TRIGGER);
    (void)adc_DoSoftwareTrigger(ADC_PHASE_IDLE);

    //    retval = (Temp[0] + Temp[1] + Temp[2] + Temp[3] + 2) / 4;
    retval = (Temp[0] + Temp[1] + 1) >> 1;
    return retval;
}

/** Convert adc bits to shunt current
 *
 * Conversion function for shunt current measurements.
 * @param[in]  u16AdcVal  Raw adc measurement (- offset)
 * @return  the shunt current in 1 mA.
 */
int16_t adc_ConvertToCurrent(uint16_t u16AdcVal)
{
    int16_t current;

    /* the current sensor current in mA units */
    current = conv_shunt_current(u16AdcVal); // Option-A
    //    current = conv_shunt_current_with_tcorrection(u16AdcVal, dBase[ADC_SAMPLE_TEMP]);	//Option-B

    return (current);
}

/** Convert adc bits to chip temperature
 *
 * @param[in]  u16AdcVal  Raw adc measurement.
 * @return  the temperature in 1 degC.
 */
int16_t adc_ConvertToTchip(uint16_t u16AdcVal)
{
    return CalculateTemperature3pLinearize(u16AdcVal);
}

/** Convert adc bits to supply voltage (Vsm filtered)
 *
 * @param[in]  u16AdcVal  Raw adc measurement.
 * @return  the filtered Vsm supply voltage in 10 mV.
 */
int16_t adc_ConvertToVsmFiltered(uint16_t u16AdcVal)
{
    return conv_motor_voltage_filtered_with_tcorrection(u16AdcVal, dBase[ADC_SAMPLE_TEMP]);
}

/** Convert adc bits to supply voltage (Vs)
 *
 * @param[in]  u16AdcVal  Raw adc measurement.
 * @return  the supply voltage in 10 mV.
 */
int16_t adc_ConvertToVsupply(uint16_t u16AdcVal)
{
    return conv_high_voltage_with_tcorrection(u16AdcVal, dBase[ADC_SAMPLE_TEMP]);
}

/** Convert adc bits to voltage (Vphase)
 *
 * @param[in]  u16AdcVal  Raw adc measurement.
 * @return  converted voltage in 1 mV.
 */
int16_t adc_ConvertToVphase(uint16_t u16AdcVal)
{
    return conv_high_voltage_with_tcorrection(u16AdcVal, dBase[ADC_SAMPLE_TEMP]);
}

/** Convert adc bits to low-IO voltage
 *
 * @param[in]  u16AdcVal  Raw adc measurement.
 * @return  converted voltage in 1 mV.
 */
int16_t adc_ConvertToVio(uint16_t u16AdcVal)
{
#define C_IO_ADC_OFFSET 0U

    u16AdcVal += C_IO_ADC_OFFSET;
    return (conv_low_voltage_with_tcorrection(u16AdcVal, dBase[ADC_SAMPLE_TEMP]) << 1U); // original
    //    return (conv_low_voltage(u16AdcVal) << 1U);		//Option-B
}

/** Convert adc bits to voltage (Vddd, ...)
 *
 * This function will convert the raw adc bits to a voltage for all
 * measurements on pin's without taking care of extra division factors.
 * @param[in]  u16AdcVal  Raw adc measurement.
 * @return  converted voltage in 1 mV.
 */
int16_t adc_ConvertToVoltage(uint16_t u16AdcVal)
{
    int16_t i16Volt;

    i16Volt = (int16_t)(mulU32_U16byU16(u16AdcVal, 660u) / 1024u);

    return (i16Volt);
}

/* ---------------------------------------------
 * Local Functions Implementation
 * --------------------------------------------- */

/**
 * @brief ADC Interrupt Service Routine
 * In case no ADC_ISR action required, this ISR has 4us overhead
 */
__attribute__((interrupt)) void _ADC_SAR_INT(void)
{
    if (p16AdcIrq != NULL)
    {
        p16AdcIrq();
    }
#ifdef APP_HAS_DEBUG
    if (p16AdcIrq2 != NULL)
    {
        p16AdcIrq2();
    }
#endif
}

/** Do a software trigger and wait for the adc to be in the next expected state
 *
 * @param[in]  NextState  the next adc state expected after the software trigger.
 * @retval  true   in case the software trigger was handled correctly.
 * @retval  false  otherwise.
 *
 * @remark  this function will also start the adc if it was not started yet.
 */
static bool adc_DoSoftwareTrigger(AdcPhaseState_t NextState)
{
    bool Result = false;

    if (IO_GET(ADC_SAR, START) == 0u)
    {
        /* ADC is not started; Start it! Prior to start, first clear any error */
        AdcClearAllErrors();
        AdcResume();
        AdcStart();
    }

    DELAY_US(C_ADC_SETTLING_TIME); /* setting time */

    AdcSwTrigger(); /* execute the software trigger */

    while (IO_GET(ADC_SAR, STOP) == 0u)
    {
        if (IO_GET(ADC_SAR, STATE) == NextState)
        {
            /* the conversion is done, adc reached the requested state */
            Result = true;
            break;
        }

        if (IO_GET(ADC_SAR, ABORTED) != 0u)
        {
            /* adc is aborted state */
            break;
        }
    }

    return (Result);
}

/* EOF */
