/**
 * @file
 * @brief The application main routines.
 * @internal
 *
 * @copyright (C) 2023 Melexis N.V.
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
 * @details This file contains the implementations of the application main routine.
 */

#include <io_map.h> /* included for easy IO access in GDB debugger*/
#include <stdint.h>
#include <stdbool.h>
#include <bist.h>
#include <plib.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys_tools.h>
#include <lib_miscio.h>
#include <builtin_mlx16.h>
#include <itc_helper.h>
#include <lib_softio.h> //250617 - Un-use code
#include <lin_api.h>
// #include <filter_lpf.h>
#include <swtimer.h>
#include <swtimer_config.h>

// 250414 - support PWM Dual Capture
// #include <pwm_capture.h>		//250617 - Un-use code
// #include <pwm_capture_config.h>
// #include <pwm_communication.h>
// #include <pwm_communication_config.h>

#include "adc.h"
#include "eeprom_app.h"
#include "lin22.h"
#include "diagnostic.h"
#include "protection.h"
#include "system.h"
#include "pwm.h"
#include "defines.h"
#include "AppLin.h"
#include "dcm_driver.h"
#include "AppValve.h"
#include "app_sensor.h"
#include "uart.h"
/* ---------------------------------------------
 * Local Constants
 * --------------------------------------------- */
/** Reference application voltage */
#define C_VSUP_REF 1200 /* 12V */
/* ---------------------------------------------
 * Local Enumerations
 * --------------------------------------------- */
/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */
uint16_t g_u16DebugData[12] = {0};
#pragma space nodp
bool g_bReset = true;
bool g_bElectricDefect = false;
bool g_bUnderVoltageDetected = false;
uint16_t g_u16MotorSupplyVoltage; /**< [10mV] */
int16_t g_i16ChipTemperature;	  /**< [C] */
uint16_t g_u16SupplyVoltage;	  /**< [10mV] */
uint16_t g_u16VBGD;				  /**< analog supply voltage [1mV] */
uint32_t l_u32MotorSupplyVoltageFilter;
uint32_t l_u32SupplyVoltageFilter;
uint32_t l_u32ChipTemperatureFilter;
uint16_t g_u16IO3;	/**< [1mV] */
uint16_t g_u16VDDA; /**< [1mV] */
int16_t g_i16ShuntCurrent;

ErrVoltage_t e8ErrorVoltage;
ErrTemp_t e8ErrorOverTemperature;
ErrShort_t e8ShortOcc;

uint16_t g_u16PositionSensor;
uint16_t u16_Angle_Gap;
uint16_t SetDuty;
uint16_t u16_LIN_Target_Angle;
uint16_t u16_CONV_Target_Angle;
uint16_t u16_Target_Angle_LCL;
uint16_t u16_Target_Angle_UCL;
uint16_t Angle_Hysteresis;
uint16_t u16_Duty_Dec_Range;

uint16_t u16_IGN_PORT_CNT;

#pragma space none

/* ---------------------------------------------
 * Local Function Declarations
 * --------------------------------------------- */
static void background_Handler(void);
static void log_reset_source(void);
static void app_Initialize(void)
{
	l_u32MotorSupplyVoltageFilter = adc_GetRawVsm() * 4u;
	l_u32SupplyVoltageFilter = adc_GetRawVs() * 4u;
	l_u32ChipTemperatureFilter = adc_GetRawTemperature() * 4u;
	swtimer_register((uint16_t)SWTIMER_MOT_CTRL_PERIOD, 1, REPETITIVE); // 100usec
	swtimer_start((uint16_t)SWTIMER_MOT_CTRL_PERIOD);
	swtimer_register((uint16_t)SWTIMER_APP_CTRL_PERIOD, 10, REPETITIVE); // 1msec
	swtimer_start((uint16_t)SWTIMER_APP_CTRL_PERIOD);

	AppLinInit();
	sensor_init();
	app_mot_init();
	AppValveInit();
}
/* ---------------------------------------------
 * Public Function Implementations
 * --------------------------------------------- */

/** Application main loop
 *
 * The application main loop. Chip and peripherals are initialized and
 * background routines will be handled in endless loop.
 *
 * \startuml width=14cm
 *   Start
 *   :Initialize the watchdog;
 *   :Initialize logger and log reset source;
 *   :Initialize IOs for debug;
 *   :Initialize the software timers;
 *   :Initialize the LIN handler;
 *   :Initialize HW protection and diagnostics (UV/OV/OTEMP/VDS/);
 *   :Initialize the motor and control handlers;
 *   :Initialize the adc (DMA);
 *   :Start application;
 *   while (True) then (yes)
 *   :Read ADC measurements (VS/VSM_TEMP/PHx/SHx);
 *   :Check protection states (OV/UV/OTEMP/VDS_OV/);
 *   :Run background handler;
 *   if (Error) then (yes)
 *   :Stop motor;
 *   else
 *   :Handle LIN motor requests (START/STOP);
 *   :Run motor background handler;
 *   endif
 *   endwhile
 *   Stop
 * \enduml
 * @return 0
 */

void globalVariableInit(void)
{
	g_u16MotorSupplyVoltage = C_VSUP_REF; /**< [10mV] */
	g_i16ChipTemperature = 35;			  /**< [C] */
	g_u16SupplyVoltage = C_VSUP_REF;	  /**< [10mV] */
	g_u16VBGD = 1185u;					  /**< analog supply voltage [1mV] */
	l_u32MotorSupplyVoltageFilter = 0u;
	l_u32SupplyVoltageFilter = 0u;
	l_u32ChipTemperatureFilter = 0u;
	g_u16IO3 = 0;  /**< [1mV] */
	g_u16VDDA = 0; /**< [1mV] */
	g_i16ShuntCurrent = 0u;
	g_u16PositionSensor = 0;
	u16_Angle_Gap = 0;
	SetDuty = 0;
	u16_LIN_Target_Angle = 3;
	u16_CONV_Target_Angle = 0;
	u16_Target_Angle_LCL = 0;
	u16_Target_Angle_UCL = 0;
	Angle_Hysteresis = 7U;
	u16_Duty_Dec_Range = 0;
	u16_IGN_PORT_CNT = 0;
}

int main(void)
{
	/* Initialize watch-dogs, both analogue and digital */
	WDG_disableIwdIt();

	/* Initialize globalVariable */
	globalVariableInit();

	/* Log reset source */
	log_reset_source();

	/* Initialize the uart module */
	uartInit();

	/* Initialize the timeout timer module */
	swtimer_init();

	/* Initialize the eeprom handler */
	eeprom_Init();

	/* initialize the pwm module */
	pwm_Init();

	/* Initialize the lin handler */
	lin22_Init();

	/* Initialize the adc driver */
	adc_Init();

	adc_Shunt_OffsetCalib();
	adc_Start(true); /* pwm must be initialized before you can use the do start and wait for loop */

	/* Start application */
	IO_SET(PORT_IO_ENABLE, IO_DISREC, 0x1E); // IO 1~4 -> ADC
	gpio_io0HvEnable();						 // 250702-1 Support IO0 for High Voltage ,ignition interface.
	/* diag & prot */
	protection_Init();

	app_Initialize();
	builtin_mlx16_enter_user_mode();
#if DEBUG_GPIO_ENABLE == 1
	softio_configureOutput(DEBUG_PIN);
#endif
	/* Application loop */
	while (1u)
	{
		WDG_conditionalAwdRefresh(); /* Restart watchdog */
		AppLinTask();

		protection_Task();
		//     fm_Atan2HelperInterpolationInlined(100,200);

		if (g_bUnderVoltageDetected) /* log UV_VS interrupt detection */
		{
			g_bUnderVoltageDetected = false; /* IC self detect*/
		}
		if (swtimer_isTriggered((uint16_t)SWTIMER_MOT_CTRL_PERIOD) != 0u) // 500s period
		{
#if DEBUG_GPIO_ENABLE == 1
			//		#if DEBUG_MODE == DEBUG_MOT_CTRL_TASK
			softio_set(DEBUG_PIN);
//		#endif
#endif
			motor_ctrl_handler();
#if DEBUG_GPIO_ENABLE == 1
			//		#if DEBUG_MODE == DEBUG_MOT_CTRL_TASK
			softio_clr(DEBUG_PIN);
//		#endif
#endif
		}
		if (swtimer_isTriggered((uint16_t)SWTIMER_APP_CTRL_PERIOD) != 0u) // 1ms period
		{
			app_motor_task();
			AppValveTask();
			uartTask();
		}

		background_Handler();
	}

	return (0); /*lint !e527 */
}

/* ---------------------------------------------
 * Callbacks
 * --------------------------------------------- */

/**
 * The routine will be called from interrupt when voltage (VS) is below 6V
 *
 * This routine can be used to (ie) save the motor position into EEPROM
 * A diode on VS, and a capacitor of min 10uF (TODO confirm min value) should be added
 *
 * \image html diode.png
 * \image latex diode.png
 */
void EVENT_UnderVoltage(void)
{
	g_bUnderVoltageDetected = true;
}

void EVENT_OverVoltage(void)
{
	//    (void)error_logger_LogError(E_ENCERRORCODES_IRQOV);
}

/**
 * This routine will be called just before going to sleep
 * @warning  callback from interrupt
 *
 * This routine can be used to (ie) save the motor position into NVRAM
 */
void EVENT_GotoSleep(void)
{

	WDG_conditionalAwdRefresh(); /* Restart watchdog */
}

/** Timer1 INT3 handler for LIN AA frame timeout */
INTERRUPT void _CTIMER0_3_INT(void)
{
	/* nop */
}

/* ---------------------------------------------
 * Local Function Implementations
 * --------------------------------------------- */

/** Background loop handler
 *
 * This function should be called from main loop. It will do some
 * background handling and it will update the signals in the
 * lin slave to master frame buffer.
 *
 * \startuml
 *   if (Temperature gradiant >= 10C) then (yes)
 *   :Recalibrate core timer;
 *   endif
 *   :Handle LIN CTRL frame (ECU-to-Node);
 *   :Handle LIN Status frame(s) (Node-to-ECU);
 *   :Call LIN handler;
 * \enduml
 *
 * Next figure shows the impact of the calibration on the swtimer period:
 *
 * \image html swtimer_calibrate.png
 * \image latex swtimer_calibrate.png
 */
static void background_Handler(void)
{
	static int16_t i16ChipTemperature = 35;

	/* Calibrate stimer over temperature */
	if (abs(g_i16ChipTemperature - i16ChipTemperature) > 10)
	{
		swtimer_calibrate(adc_GetRawTemperature());
		i16ChipTemperature = g_i16ChipTemperature;
	}
}

/**
 * Log reset source
 */
static void log_reset_source(void)
{
	uint16_t temp;
	if (IO_GET(PORT_MISC_IN, LOCAL_WU) != 0u)
	{
		/* IO_0 wake up */
	}
	else if (IO_GET(PORT_MISC_IN, LIN_WU) != 0u)
	{
		/* LIN wake up */
	}
	else if (IO_GET(RST_CTRL, AWD_WBOOT) != 0u)
	{
		/* watch dog reset */
	}
	else
	{

		/* power on reset */
	}
#if LIN_WAKEUP_DISABLE == 1
	for (temp = 32; temp > 0; temp--)
	{

		if (softio_get(0) == 0)
		{

			u16_IGN_PORT_CNT++;
			if (u16_IGN_PORT_CNT >= 16)
			{
				lin22_GotoSleep();
			}
		}
		else
		{
		}
	}
#endif
#if 0	
    uint16_t AWD_WBOOT : 1;
    uint16_t SOFT_WBOOT : 1;
    uint16_t HVDIG_WBOOT : 1;
    uint16_t DBG_WBOOT : 1;
    uint16_t IWD_WBOOT : 1;
    uint16_t reserved_5 : 8;
    uint16_t SOFT_RESET : 1;
    uint16_t HVDIG_USED : 1;
    const uint16_t HVDIG_OK : 1;
#endif
}

/* EOF */
