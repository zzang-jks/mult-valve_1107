/**
 * @file
 * @brief Protections detection handling
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

#include <plib.h>
#include <stdbool.h>
#include <swtimer.h>
#include "diagnostic.h"
#include "protection.h"
#include "defines.h"
#include "AppValve.h"
/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */

#pragma space nodp
ErrVoltage_t g_e8ErrorVoltage = C_ERR_VOLTAGE_IN_RANGE; /**< Protection: voltage */
ErrTemp_t g_e8ErrorOverTemperature = C_ERR_TEMP_NO;     /**< Protection: over-temperature */
ErrShort_t g_e8ShortOcc = C_ERR_SHORT_NO;               /**< Protection: short-circuit */
uint8_t g_e8OverCurrent = 0;                            /**< Protection:  */

#pragma space none

/* ---------------------------------------------
 * Public Functions
 * --------------------------------------------- */

/**
 * Initialize the library
 *
 * @warning this module is depending of the module Diagnostic, which must be initialised upfront
 */
void protection_Init(void)
{
    g_e8ErrorVoltage = C_ERR_VOLTAGE_IN_RANGE;
    g_e8ErrorOverTemperature = C_ERR_TEMP_NO;
    g_e8ShortOcc = C_ERR_SHORT_NO;
    g_e8OverCurrent = 0;
    swtimer_register(SWTIMER_PROTECTION_DELAY_VSM, (125 * 10u), SINGLE_SHOT); /*125msec*/
    swtimer_register(SWTIMER_PROTECTION_DELAY_OVT, (10 * 10u), SINGLE_SHOT);  /*10msec*/

    diagnostic_Init();
    diagnostic_Start();
}

/**
 * Release resources
 */
void protection_Close(void)
{
    diagnostic_Close();
}

/**
 * (Re-)Start Protections
 *
 * Should be recalled to (re-)enable the pre-driver again after a VDS or Short-circuit occurred
 */
void protection_Start(void)
{
    g_e8ShortOcc = C_ERR_SHORT_NO;
    diagnostic_Start();
}

/**
 * Stop Protections
 */
void protection_Stop(void)
{
}

/**
 * Check for Short-circuits (VDS, OC)
 *
 * \startuml {protection_checkShort.png} "Protection CheckShort" width=7cm
 *   Start
 *   if (Over-current) then (yes)
 *   :Connect the phase to Tristate;
 *   note right:  Hardware protection (DTI<1us);
 *   :Set g_e8ShortOcc to 'C_ERR_SHORT_OC';
 *   else if (VDS(HS) error detected) then (yes)
 *   :Connect the phase to GND;
 *   note right:  Hardware protection (DTI<1us);
 *   :Set g_e8ShortOcc to 'C_ERR_SHORT_VDS';
 *   endif
 *   Stop
 * \enduml
 */
void protection_CheckShort(void)
{
    if (diagnostic_IsOVCPending())
    {

        g_e8OverCurrent = 1;
    }
    else
    {
        if (g_e8OverCurrent == 1)
        {
        }
        //		g_e8OverCurrent = 0;
    }
    if (diagnostic_IsVDSPending())
    {
        g_e8ShortOcc = C_ERR_SHORT_VDS;
    }
    else
    {
        if (g_e8ShortOcc == C_ERR_SHORT_VDS)
        {
        }
        //        g_e8ShortOcc = C_ERR_SHORT_NO;
        /* MISRA C-2012 Rule 15.7 */
    }
}

/**
 * Check for Under- Over- voltage
 *
 * \startuml {protection_checksupplymotorvoltage.png} "Protection CheckSupplyMotorVoltage" width=10cm
 *   Start
 *   if (VSM above 22V) then (yes)
 *   :Connect the phase to GND;
 *   note right:  Hardware protection (DTI<1us);
 *   :Set g_e8ErrorVoltage to 'C_ERR_VOLTAGE_OVER_HW';
 *   else if (VSM < 6V) then (yes)
 *   :Call application callback 'EVENT_UnderVoltage';
 *   note right:  Via Interrupt (DTI<50us);
 *   :Set g_e8ErrorVoltage to 'C_ERR_VOLTAGE_UNDER_HW';
 *   else if (VSM > 16V(+hyst) for more than 125ms) then (yes)
 *   :Set g_e8ErrorVoltage to 'C_ERR_APPL_OVER_VOLT';
 *   else if (VSM < 8V(-hyst) for more than 125ms) then (yes)
 *   :Set g_e8ErrorVoltage to 'C_ERR_APPL_UNDER_VOLT';
 *   else
 *   :Set g_e8ErrorVoltage to 'C_ERR_VOLTAGE_IN_RANGE';
 *   endif
 *   Stop
 * \enduml
 *
 * @param[in]  a_u16SupplyMotorVoltage  VSM voltage [10mV]
 */
void protection_CheckSupplyMotorVoltage(void)
{
    if (diagnostic_IsOVPending())
    {
        g_e8ErrorVoltage = C_ERR_VOLTAGE_OVER_HW;
    }
    else if (diagnostic_IsUVPending())
    {

        g_e8ErrorVoltage = C_ERR_VOLTAGE_UNDER_HW;
    }
    else
    {
        if ((g_e8ErrorVoltage == C_ERR_VOLTAGE_OVER_HW) && (get_valve_voltage() < C_APPL_OVOLT))
        {
            IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 1u); /* clear MEM flag */
            IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 0u); /* re-enable hardware protection */
        }
        if ((g_e8ErrorVoltage == C_ERR_VOLTAGE_UNDER_HW) && (get_valve_voltage() > C_APPL_UVOLT))
        {
            IO_SET(PORT_DRV1_PROT, DIS_UV_VS, 1u); /* clear MEM flag */
            IO_SET(PORT_DRV1_PROT, DIS_UV_VS, 0u); /* re-enable hardware protection */
        }
        //		g_e8ErrorVoltage = C_ERR_VOLTAGE_IN_RANGE;
    }
}

/**
 * Check for Chip Over-Temperature
 *
 * The hardware temperature threshold is fixed by design. Software thresholds and timings are application dependent.
 *
 * \startuml {protection_CheckChipTemperature.png} "Protection CheckChipTemperature" width=14cm
 *   Start
 *   if (Chip temperature > 185C) then (yes)
 *   :Set g_e8ErrorOverTemperature state to 'C_ERR_TEMP_ALARM';
 *   else if (Chip temperature > C_APPL_OTEMP(+hyst) for more than 3s) then (yes)
 *   :Set g_e8ErrorOverTemperature state to 'C_ERR_TEMP_WARNING';
 *   else if (Chip temperature < C_APPL_OTEMP(-hyst)) then (yes)
 *   :Set g_e8ErrorOverTemperature state to 'C_ERR_TEMP_NO';
 *   endif
 *   Stop
 * \enduml
 *
 * @param[in]  a_i16ChipTemperature  chip internal temperature sensor [C]
 */
void protection_CheckChipTemperature(void)
{
    if (diagnostic_IsOVTPending())
    {

        g_e8ErrorOverTemperature = C_ERR_TEMP_ALARM;
        //            swtimer_stop(SWTIMER_PROTECTION_DELAY_OVT);
    }
    else
    {
        if ((g_e8ErrorOverTemperature == C_ERR_TEMP_ALARM) && (get_valve_temperature() < (int16_t)(C_APPL_OTEMP + C_TEMP_CONV_OFFSET)))
        {
            IO_SET(PORT_DRV1_PROT, DIS_OVT, 1u); /* clear MEM flag */
            IO_SET(PORT_DRV1_PROT, DIS_OVT, 0u); /* re-enable hardware protection */
        }
        //	 g_e8ErrorOverTemperature = C_ERR_TEMP_NO;
        /* MISRA C-2012 Rule 15.7 */
    }
}

void protection_Task(void)
{
    protection_CheckShort();

    protection_CheckSupplyMotorVoltage();
    protection_CheckChipTemperature();
}
/* EOF */
