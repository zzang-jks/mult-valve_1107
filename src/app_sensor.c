/*
 * app_sensor.c
 *
 *  Created on: 2025. 5. 29.
 *      Author: mctp
 */
/* platform */
#include <stdint.h>
#include <stdbool.h>
#include <plib.h>
#include <itc_helper.h>
#include <sys_tools.h>
#include <io.h>
#include <io_map.h>
// #include <filter_lpf.h>
#include <filter_avg.h>
#include <conv_shunt_current.h>
// #include <mathlib.h>
/* application */
#include "defines.h"
#include "adc.h"
#include "app_sensor.h"
#include "dcm_driver.h"

static uint16_t l_au16MotorOffsetCurrent;
static uint16_t l_au16SupplyAvgBuffer[8u];		/**< supply voltage filter (buffer) */
static uint16_t l_au16TemperatureAvgBuffer[8u]; /**< Temperature filter (buffer) */
static uint16_t l_au16CurrentAvgBuffer[8u];		/**< Motor current filter (buffer) */
static uint16_t l_au16VddaAvgBuffer[8u];		/**< Vdda filter (buffer) */
static uint16_t l_au16IgnitionAvgBuffer[8u];	/**< supply voltage filter (buffer) */
static uint16_t l_au16GmrIO1AvgBuffer[8u];		/**  */
static uint16_t l_au16GmrIO2AvgBuffer[8u];		/** */
static uint16_t l_au16GmrIO3AvgBuffer[8u];		/**  */
static uint16_t l_au16GmrIO4AvgBuffer[8u];		/** */
static FILTER_AVG_Object_t l_sAdcAvgObject[] = {
	{0u, 0u, &l_au16SupplyAvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16TemperatureAvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16CurrentAvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16VddaAvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16IgnitionAvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16GmrIO1AvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16GmrIO2AvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16GmrIO3AvgBuffer[0], 8u, 0u},
	{0u, 0u, &l_au16GmrIO4AvgBuffer[0], 8u, 0u}};

int16_t l16_GmrCalEnable = 0;
int16_t l16_SinPosMaxPeak;
int16_t l16_SinPosMinPeak;
int16_t l16_SinNegMaxPeak;
int16_t l16_SinNegMinPeak;
int16_t l16_CosinPosMaxPeak;
int16_t l16_CosinPosMinPeak;
int16_t l16_CosinNegMaxPeak;
int16_t l16_CosinNegMinPeak;
int16_t l16_SinOutputOffset;
int16_t l16_CosinOutputOffset;
int16_t l16_SetGmrSensorOffset = 0;
void sensor_init(void)
{
	uint16_t num = 0, index = 0, initValue;

	for (num = 0; num < C_ADC_END_ITEM; num++)
	{
		if (num == C_ADC_VS_)
			initValue = 0xFF;
		else if (num == C_ADC_TEMP_)
			initValue = 0x260;
		else
			initValue = 0;
		l_sAdcAvgObject[num].u32MovAvgxN = 0;
		for (index = 0; index < 8; index++)
		{
			l_sAdcAvgObject[num].pu16Raw[index] = initValue;
			l_sAdcAvgObject[num].u32MovAvgxN += initValue;
		}
	}
	gmr_calibration_setup();
	//	l16_SetGmrSensorOffset=C_GMR_SENSOR_OFFSET*C_GMR_ANGLE_SCALE_FACTOR;
	l16_SetGmrSensorOffset = (DEFAULT_GMR_OFFSET * C_GMR_ANGLE_SCALE_FACTOR);
	l_au16MotorOffsetCurrent = 0;
}
void gmr_calibration_setup(void)
{
	l16_GmrCalEnable = 1;
	l16_SinPosMaxPeak = 0;
	l16_SinPosMinPeak = 0x7fff;
	l16_SinNegMaxPeak = 0;
	l16_SinNegMinPeak = 0x7fff;
	l16_CosinPosMaxPeak = 0;
	l16_CosinPosMinPeak = 0x7fff;
	l16_CosinNegMaxPeak = 0;
	l16_CosinNegMinPeak = 0x7fff;
	l16_SinOutputOffset = 0xFFFF;
	l16_CosinOutputOffset = 1;
}
void gmr_calibration_end(void)
{
}
void set_gmr_sensor_offset(int16_t offset)
{
	l16_SetGmrSensorOffset = offset;
}
int16_t get_gmr_sensor_offset(void)
{
	return l16_SetGmrSensorOffset;
}
void adc_raw_update(void)
{
	uint16_t index = 0;

	for (index = 0; index < C_ADC_END_ITEM; index++)
	{
		switch (index)
		{
		case C_ADC_VS_:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_VS_], adc_GetRawVs());
			//						temp = l_sAdcAvgObject[C_ADC_VS_].u16MovAvg;
			break;
		case C_ADC_TEMP_:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_TEMP_], adc_GetRawTemperature());
			//						temp = l_sAdcAvgObject[C_ADC_TEMP_].u16MovAvg;
			break;
		case C_ADC_CURRENT_:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_CURRENT_], adc_GetRawCurrent());
			//						temp = l_sAdcAvgObject[C_ADC_CURRENT_].u16MovAvg;
			break;
		case C_ADC_VDDA:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_VDDA], adc_GetRawVdda());
			//						temp = l_sAdcAvgObject[C_ADC_VDDA].u16MovAvg;
			break;
		case C_ADC_IGN:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_IGN], adc_GetRawIGN());
			//						temp = l_sAdcAvgObject[C_ADC_VREF_].u16MovAvg;
			break;
		case C_ADC_SENSOR_1:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_SENSOR_1], adc_Get_GMR_nCosine());
			//						temp = l_sAdcAvgObject[C_ADC_SENSOR_].u16MovAvg;
			break;
		case C_ADC_SENSOR_2:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_SENSOR_2], adc_Get_GMR_nSine());
			//						temp = l_sAdcAvgObject[C_ADC_SENSOR_].u16MovAvg;
			break;
		case C_ADC_SENSOR_3:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_SENSOR_3], adc_Get_GMR_pCosine());
			//						temp = l_sAdcAvgObject[C_ADC_SENSOR_].u16MovAvg;
			break;
		case C_ADC_SENSOR_4:
			FILTER_AVG_CalcMovAvg(&l_sAdcAvgObject[C_ADC_SENSOR_4], adc_Get_GMR_pSine());
			//						temp = l_sAdcAvgObject[C_ADC_SENSOR_].u16MovAvg;
			break;
		default:
			break;
		}
	}
}

int16_t get_sensor_raw_data(uint16_t num)
{
	return (l_sAdcAvgObject[num].u16MovAvg);
}
uint16_t get_conv_vdda_voltage(void)
{
	return adc_ConvertToVoltage(get_sensor_raw_data(C_ADC_VDDA));
}
uint16_t get_conv_supply_voltage(void)
{
	return adc_ConvertToVsupply(get_sensor_raw_data(C_ADC_VS_));
}
int16_t get_conv_ic_temperature(void)
{
#if 0
	return adc_ConvertToTchip(get_sensor_raw_data(C_ADC_TEMP_));
#else
	int16_t temperature = adc_ConvertToTchip(get_sensor_raw_data(C_ADC_TEMP_));
	if (temperature >= 0)
	{
		temperature += C_TEMP_CONV_OFFSET;
	}
	else
	{
		if (temperature < (-40))
		{
			temperature = 0;
		}
		else
		{
			temperature = (C_TEMP_CONV_OFFSET + temperature);
		}
	}

	return temperature;
#endif
}
uint16_t get_conv_mot_current(void)
{
	return adc_ConvertToCurrent(get_sensor_raw_data(C_ADC_CURRENT_));
}
/*
6.5 = 143
7V = 152
9V = 183
12V= 211 = 12V
18V= 242
*/
#define IGN_CONV_MAP_SIZE 4u
static const conversion_t IGNconversionMap[IGN_CONV_MAP_SIZE] = {
	{0x132, 650u},
	{0x15B, 900u},	// 1
	{0x178, 1200u}, // 2
	{0x19B, 1800u}	// 3
};
uint16_t get_conv_ignition_voltage(void)
{
	uint16_t volt = get_sensor_raw_data(C_ADC_IGN);
	uint16_t cal_result = 0, index;

	if (volt < IGNconversionMap[0].actual)
	{
		cal_result = 0;
	}
	else if (volt > IGNconversionMap[3].actual)
	{
		cal_result = 1800u;
	}
	else
	{
		for (index = 0U; index < (IGN_CONV_MAP_SIZE - 1U); index++)
		{
			if (volt <= IGNconversionMap[index + 1U].actual)
			{
				break;
			}
		}
		cal_result = forward_linear_Interpolation(volt,
												  IGNconversionMap[index].actual, IGNconversionMap[index + 1U].actual,
												  IGNconversionMap[index].correction, IGNconversionMap[index + 1U].correction);
	}

	return (cal_result);
}
/*
pSin : 0x237(max) 0x2d(min)
nSin : 0x3FF(max) 0x59(min)
pSin-nSin : 0x2BA(max) 0xFD42(min)  (5V)
pSin-nSin : 0x219(max) 0xFDE0(min)  (3.3V)
l16_SinOutputOffset = 0xFFFF
*/
int16_t get_gmr_sine_output(void)
{
	int16_t cal, offset = C_GMR_OUTPUT_OFFSET;
	int16_t pSin, nSin;
	pSin = get_sensor_raw_data(C_ADC_SENSOR_4) - offset;
	nSin = get_sensor_raw_data(C_ADC_SENSOR_2) - offset;

	cal = pSin - nSin;
#if 0	
if (cal>l16_SinPosMaxPeak	)
{
l16_SinPosMaxPeak=cal;
}
if (cal<l16_SinNegMaxPeak)
{
l16_SinNegMaxPeak=cal;
}
#endif
	cal = cal - l16_SinOutputOffset;

	if (cal > C_GMR_POSITIVE_MAX)
	{
		cal = C_GMR_POSITIVE_MAX;
	}
	else if (cal < C_GMR_NEGAITIVE_MAX)
	{
		cal = C_GMR_NEGAITIVE_MAX;
	}
	else
	{
	}
	return (cal);
}
/*
pCosin : 0x3FF(max) 0x3d(min)
nCosin : 0x3FF(max) 0x77(min)
pCosin-nCosin : 0x2BD(max) 0xFD44(min)  5V
pCosin-nCosin : 0x21B(max) 0xFDE5(min)  3.3V
l16_CosinOutputOffset=0;
*/
int16_t get_gmr_cosine_output(void)
{
	int16_t cal, offset = C_GMR_OUTPUT_OFFSET;
	int16_t pCosin, nCosin;
	pCosin = get_sensor_raw_data(C_ADC_SENSOR_3) - offset;
	nCosin = get_sensor_raw_data(C_ADC_SENSOR_1) - offset;

	cal = pCosin - nCosin;
#if 0		
if (cal>l16_CosinPosMaxPeak	)
{
l16_CosinPosMaxPeak=cal;
}
if (cal<l16_CosinNegMaxPeak)
{
l16_CosinNegMaxPeak=cal;
}
#endif
	cal = cal - l16_CosinOutputOffset;

	if (cal > C_GMR_POSITIVE_MAX)
	{
		cal = C_GMR_POSITIVE_MAX;
	}
	else if (cal < C_GMR_NEGAITIVE_MAX)
	{
		cal = C_GMR_NEGAITIVE_MAX;
	}
	else
	{
	}
	return (cal);
}
/* 0~0xFFFF -> 0~360*10*/
static uint16_t MLX_to_GMR_conv(uint16_t angle)
{
	uint16_t cal, offset;
	if (angle < 0x2000) /* 45*/
	{
		offset = 0;
	}
	else if (angle < 0x4000) /* 90*/
	{
		angle -= 0x2000;
		offset = 450;
	}
	else if (angle < 0x6000) /* 135*/
	{
		angle -= 0x4000;
		offset = 900;
	}
	else if (angle < 0x8000) /* 180*/
	{
		angle -= 0x6000;
		offset = 1350;
	}
	else if (angle < 0xA000) /* 225*/
	{
		angle -= 0x8000;
		offset = 1800;
	}
	else if (angle < 0xC000) /* 270*/
	{
		angle -= 0xA000;
		offset = 2250;
	}
	else if (angle < 0xE000) /* 315*/
	{
		angle -= 0xC000;
		offset = 2700;
	}
	else /* 360*/
	{
		angle -= 0xE000;
		offset = 3150;
	}
	angle = (angle >> 6); /* div 64 */
	cal = (450 * angle) >> 7;
	cal += offset;
	if (cal >= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT)
	{
		cal -= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
	}

	cal += l16_SetGmrSensorOffset;
	if (cal >= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT)
	{
		cal -= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
	}

	return (cal);
}
#if 0
        g_u16IOADC1 = adc_Get_GMR_nSine();
        g_u16IOADC2 = adc_Get_GMR_pSine();
        g_u16IOADC3 = adc_Get_GMR_nCosine();
        g_u16IOADC4 = adc_Get_GMR_pCosine();

        int16_t g_i16GMR_Position;
        g_i16GMR_Position = (int16_t)atan2I16(g_u16IOADC2, g_u16IOADC4);

/** Signed atan2 function
 *
 *  This function calculates (an approximation of) arctan (y / x) where y and x
 *  are two 16 bit integer numbers.
 *  The result is an angle value between 0 and 2&pi;, represented as a 16 bit
 *  signed fraction of 2&pi; i.e. Q15 format.
 *
 *  y (signed)  | x (signed)  | atan2 (y/x) | Equivalent angle (rad)
 *  ----------- | ----------- | ----------- | ----------------------
 *  0           | 0           | 0x0000      | 0
 *  0           | 1           | 0x0000      | 0
 *  1           | 1           | 0x2000      | &pi;/4
 *  1           | 0           | 0x4000      | &pi;/2
 *  1           | -1 (0xFFFF) | 0x6000      | 3&pi;/4
 *  0           | -1 (0xFFFF) | 0x8000      | &pi;
 *  -1 (0xFFFF) | -1 (0xFFFF) | 0xA000      | 5&pi;/4
 *  -1 (0xFFFF) | 0           | 0xC000      | 3&pi;/2
 *  -1 (0xFFFF) | 1           | 0xE000      | 7&pi;/4
 *  0                  1              0xFFFF
 *  @note       atan2I16(0, 0) is definded as 0.
 *
 *  @param[in]  y     Y coordinate
 *  @param[in]  x     X coordinate
 *
 *  @return     The output angle, as 16 bit signed fraction of 2&pi;.
 *             32768(-1->-&pi;) < out < 32768(1->&pi;)
 *  @ingroup    math_trigonometry_group
 */
#endif
/* ELMOS
u16 loc_rotor_angle_estimated = math_get_angle_unsafe( loc_e_beta_filtered, loc_e_alpha_filtered );
*/

int16_t calculate_gmr_angle(void)
{
	int16_t ang_result;

	ang_result = (int16_t)atan2I16(get_gmr_cosine_output(), get_gmr_sine_output());

	/* output angle(0~0xFFFF) -> 0~360 degree */
	ang_result = MLX_to_GMR_conv(ang_result);

	return ang_result;
}

int16_t forward_linear_Interpolation(int16_t x, int16_t x0, int16_t x1, int16_t y0, int16_t y1)
{
	int16_t tmp = 0;
	int16_t retVal = 0;

	tmp = (int16_t)(y1 - y0) * (int16_t)(x - x0);

	if ((x1 - x0) != 0)
	{
		tmp /= (int16_t)(x1 - x0);
	}

	tmp += (int16_t)y0;

	retVal = tmp;

	return retVal;
}
int16_t reverse_linear_Interpolation(int16_t x, int16_t x0, int16_t x1, int16_t y0, int16_t y1)
{
	int16_t tmp = 0;
	int16_t retVal = 0;

	tmp = (int16_t)(y1 - y0) * (int16_t)(x0 - x);
	if ((x0 - x1) != 0)
	{
		tmp /= (int16_t)(x0 - x1);
	}
	tmp += (int16_t)y0;

	retVal = tmp;
	return retVal;
}
