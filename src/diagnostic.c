/**
 * @file
 * @brief Diagnostic (HW protections) handling.
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

#include <stdint.h>
#include <stdbool.h>
#include <plib.h>
#include <itc_helper.h>
#include <sys_tools.h>
#include "diagnostic.h"

#include "adc.h"
#include <lib_softio.h>
#include <eeprom_parameters.h>
/* ---------------------------------------------
 * Local types
 * --------------------------------------------- */

#define PROTECTION_MODE_TRISTATE 1 /**< highZ state after diagnostic event */

/* ---------------------------------------------
 * Local functions
 * --------------------------------------------- */

/**
 * Event called when under voltage is detected
 */
__attribute__((weak)) void EVENT_UnderVoltage(void)
{
}

/* ---------------------------------------------
 * Global Variables
 * --------------------------------------------- */

/* ---------------------------------------------
 * Public Functions
 * --------------------------------------------- */

/**
 * Enable motor-driver automatically shut-off in case of over-current or VDS or over-voltage.
 * Don't automatically shut-off motor-driver on under-voltage, or over-temperature.
 *
 * @warning should be called in system mode
 *
 */
void diagnostic_Init(void)
{
    /* Disable hardware protection for over-temperature, under-voltage
     * Hardware protection is enabled in case of OC, VDS(LS), VDS(HS), OV_VS, OVT
     */
    IO_SET(PORT_DRV1_PROT, DIS_UV_VDDA, 1u, DIS_UV_VDDAF, 1u, DIS_UV_VS, 1u);

    /*
     * Connect the phase to GND in case of VDS(HS),
     */
    IO_SET(PORT_DRV1_PROT,
           OVT_PM,
           0, /* low */
           OV_VS_PM,
           0, /* low */
           OV_HS_VDS_PM,
           0, /* low */
           OV_LS_VDS_PM,
           0, /* high */
           OC_PM,
           PROTECTION_MODE_TRISTATE);

    /* Set/enable UV & OV debounce circuitry */
    IO_SET(PORT_SUPP_CFG, UV_VS_FILT_SEL, 1u); /* 0 : 1-2us filtering, 1 : 100-110us filtering */
    IO_SET(PORT_SUPP_CFG, OV_VS_FILT_SEL, 1u); /* 0 : 1-2us filtering, 1 : 100-110us filtering */

    /* Under/Over voltage detection levels */
    IO_SET(PORT_MISC_OUT, PRUV_VS, vsuv6V);
    //    IO_SET(PORT_MISC_OUT, PRUV_VS, vsuv9V);
    //    IO_SET(PORT_MISC_OUT, PROV_VS, vsov22V);
    IO_SET(PORT_MISC_OUT, PROV_VS, vsov24V);

    uint16_t u16trim_high = EE_GET(TRIM_CSA_CL_HIGH); // 2A Threshold from calibration data
    uint16_t u16trim_low = EE_GET(TRIM_CSA_CL_LOW);   // 1.5A Trheshold from calibration data

    IO_SET(TRIM3_DRV, TRIM_CSA_CL, u16trim_high); // HW OC threshold ,2A

#if 0
    // Change LIN Slew rate
    // init in lin22.c
//#define slewrate_option_0	0	// 100%, default
//#define slewrate_option_1	1	// 87.5%
//#define slewrate_option_2	2	// 125%
//#define slewrate_option_3	3	// 112.5%
//#define slewrate_option_4	4	// 50%%
//#define slewrate_option_5	5	// 37.5%
//#define slewrate_option_6	6	// 75%%
//#define slewrate_option_7	7	// 62.5%

    //    IO_SET(TRIM_RCO1M, PRE_TR_LIN_SLEWRATE, slewrate_option_0);

    // Change Phase otuput(U/V/W/T) slew rate
    IO_SET(TRIM2_DRV, TRIM_SLWRT, PHASE_SLEWRATE_OPTION_0);

    // CPU CLOCK Spread spectrum modulation
    // CPU Clock modulation frequency : 73Khz
    // CPU Modulation Index : 9.2
    IO_SET ( PORT_STEP_CONF ,
    		STEP_CNT , 32U,	 	/* STEP_CNT = 32 */
			STEP_DUR , 12U, 	/* STEP_DUR = 12 */
			STEP_INC , 1U) ; 	/* STEP_INC = 1 */
    IO_SET (PORT_SSCM_CONF, SSCM_EN , 1u ) ; /* Enable the spread spectrum modulation */

    // Charge pump Spread spectrum modulation
    // CP Clock modulation frequency : 87Khz
    // CP Modulation Index : 33
    IO_SET ( PORT_STEP2_CONF ,
    		STEP2_CNT , 138U, 	/* STEP_CNT = 138 */
			STEP2_DUR , 5U, 	/* STEP_DUR = 5 */
			STEP2_INC , 1U) ; 	/* STEP_INC = 1 */
    IO_SET (PORT_SSCM2_CONF , SSCM2_EN , 1u ) ; /* Enable the spread spectrum modulation */
#endif
    // VDDA output voltage level
    // Default : 3.3V output
    IO_SET(PORT_MISC_OUT, SWITCH_VDDA_TO_5V, VDDA_OUTPUT_LEVEL); /* 5V output*/
#if 0
    /* Enable Diagnostic interrupt */
    Itc_Enable(UV_VS);		//Under Voltage_ISR Enable
    Itc_Enable(OV_VS);		//Over Voltage Enable
    Itc_Enable(OVC);		//Over Current_ISR Enable
    Itc_Enable(OVT);		//Over Temperature_ISR Enable
#endif
}

/**
 * Disable driver: disable all interrupts and HW protections
 *
 */
void diagnostic_Close(void)
{
    /* Do not use pre-driver when diagnostics are disabled */
    IO_SET(PORT_DRV2_PROT, DIS_DRV, 1u);

    /* Disable interrupts */
    ENTER_SECTION(ATOMIC_SYSTEM_MODE);
    Itc_Disable(UV_VS);
    EXIT_SECTION();
}

/**
 * Clear over current, VDS and OV pending flag(s)
 */
void diagnostic_Start(void)
{
    IO_SET(PORT_DRV1_PROT, DIS_OC, 1u); /* clear MEM flag */
    IO_SET(PORT_DRV1_PROT, DIS_OC, 0u); /* re-enable hardware protection */

    IO_SET(PORT_DRV1_PROT, DIS_UV_VS, 1u); /* clear MEM flag */
    IO_SET(PORT_DRV1_PROT, DIS_UV_VS, 0u); /* re-enable hardware protection */

    IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 1u); /* clear MEM flag */
    IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 0u); /* re-enable hardware protection */

    IO_SET(PORT_DRV1_PROT, DIS_OVT, 1u); /* clear MEM flag */
    IO_SET(PORT_DRV1_PROT, DIS_OVT, 0u); /* re-enable hardware protection */
}
/**
 * Clear over current, VDS and OC pending flag(s)
 */
void DIAGNOSTIC_Reset(void)
{
    IO_SET(PORT_DRV1_PROT, DIS_OC, 1u); /* clear OVC_MEM flag */
    IO_SET(PORT_DRV1_PROT, DIS_OC, 0u); /* re-enable OVC hardware protection */

    IO_SET(PORT_DRV1_PROT, DIS_OV_HS_VDS, 1u); /* clear OV_HS_VDS_MEM flag */
    IO_SET(PORT_DRV1_PROT, DIS_OV_HS_VDS, 0u); /* re-enable HS-VDS hardware protection */
}

/**
 * Read UV diagnostic status. Re-enable the UV interrupt in case error is gone.
 * @return true in case of UV on VS
 */
bool diagnostic_IsUVPending(void)
{
    bool bRetVal;

    bRetVal = IO_GET(PORT_SUPP_IN, UV_VS_IT) ? true : false;
#if 0
    /* Re-enable interrupt when voltage is above threshold */
    if (!bRetVal)
    {
        ENTER_SECTION(ATOMIC_SYSTEM_MODE);
        Itc_Enable(UV_VS);
        EXIT_SECTION();
    }
#endif
    return bRetVal;
}

/**
 * Read Diagnostic status
 * @return true in case of OV on VS
 */
bool diagnostic_IsOVPending(void)
{
    bool bRetVal;

    bRetVal = IO_GET(PORT_DIAG_IN, OV_VS_MEM) ? true : false;
#if 0
    if (bRetVal)
    {
        IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 1u);  /* clear MEM flag */
        IO_SET(PORT_DRV1_PROT, DIS_OV_VS, 0u);  /* re-enable hardware protection */
    }
#endif
    return bRetVal;
}

/**
 * Read Diagnostic status
 * @return true in case of over temperature
 */
bool diagnostic_IsOVTPending(void)
{
    bool bRetVal;

    bRetVal = IO_GET(PORT_DIAG_IN, OVT_MEM) ? true : false;
#if 0
    if (bRetVal)
    {
        IO_SET(PORT_DRV1_PROT, DIS_OVT, 1u);  /* clear MEM flag */
        IO_SET(PORT_DRV1_PROT, DIS_OVT, 0u);  /* re-enable hardware protection */
    }
#endif
    return bRetVal;
}

/**
 * Read Diagnostic status
 * @return true in case of one over-current did occurred
 */
bool diagnostic_IsOVCPending(void)
{
    bool bRetVal;

    bRetVal = IO_GET(PORT_DIAG_IN, OVC_MEM) ? true : false;

    return bRetVal;
}

/**
 * Read Diagnostic status
 * @return true in case of one VDS error  did occurred
 */
bool diagnostic_IsVDSPending(void)
{
    bool bRetVal;

    bRetVal = (IO_GET(PORT_DIAG_IN, OV_HS_VDS_MEM) || IO_GET(PORT_DIAG_IN, OV_LS_VDS_MEM)) ? true : false;

    return bRetVal;
}

/* ---------------------------------------------
 * Local Functions
 * --------------------------------------------- */

/**
 * Under voltage interrupt
 */
INTERRUPT void _UV_VS_INT(void)
{
    /* Disable the interrupt, otherwise the interrupt will be called continuously */
    Itc_Disable(UV_VS);

    /* Call application callback (save data to EEPROM, log error, etc) */
    EVENT_UnderVoltage();
}

/**
 * Over voltage interrupt
 */
INTERRUPT void _OV_VS_INT(void)
{

    /* Disable the interrupt, otherwise the interrupt will be called continuously */
    Itc_Disable(OV_VS);

    /* Call application callback (save data to EEPROM, log error, etc) */
    //    EVENT_OverVoltage();
}

/**
 * Over Temperature interrupt
 */
INTERRUPT void _OVT_INT(void)
{

    /* Disable the interrupt, otherwise the interrupt will be called continuously */
    Itc_Disable(OVT);
}

/**
 * Over Current interrupt
 */
INTERRUPT void _OVC_INT(void)
{

    /* Disable the interrupt, otherwise the interrupt will be called continuously */
    Itc_Disable(OVC);
}

/* EOF */
