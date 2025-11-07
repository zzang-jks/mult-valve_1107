/*
 * AppValve.c
 *
 *  Created on: 2025. 5. 27.
 *      Author: mctp
 */
#include <stdint.h>
#include <stdbool.h>
#include <lin_api.h>
#include "defines.h"
#include "AppValve.h"
#include "adc.h"
#include "app_sensor.h"
#include "dcm_driver.h"
#include "AppLin.h"
#include "lin22.h"
#include "eeprom_app.h"

tProtectCondition u16EventState = NONE_ERROR;
uint16_t u16EventValue = 0;

/* local variables */
struct
{
	tValveState state;	   /* current state */
	tValveState lastState; /* last state */
	uint16_t linLiveTimeOut;
	uint16_t elapsedTime;
	uint16_t ignOffCnt;
	uint8_t sleepState;
	uint8_t initStatus;
	uint16_t motorMotion;
	struct
	{
		int16_t targetAngle;  /*  */
		int16_t currentAngle; /*  */
		int16_t modeAngle[2]; /*calibrated  */
		uint8_t fault;
		uint8_t retryCnt;
	} pos;

	struct
	{
		uint8_t state; /* calibration state */
		uint8_t offsetDone;
		uint8_t req2Cal;   /* Internal calibration request 0:no request / 1:request / 2:calib done*/
		uint8_t req1Cal;   /* Internal calibration request 0:no request / 1:request / 2:calib done*/
		int16_t travel;	   /*  */
		int16_t d0Angle;   /*  */
		int16_t d360Angle; /*  */
		uint16_t delay;
		uint16_t timer;
	} calibration;

	struct
	{
		uint16_t timeOut;
		uint8_t targetMode; /* target position  */
		uint8_t Enable;
		uint8_t Initial;	/* t  */
		uint8_t ForcedDiag; /*  */
		uint8_t lastMode;
		uint8_t actualMode; /* actual position  */
		uint8_t moving;
		uint8_t faultMode;
	} comm; /* communication parameter */

	struct
	{
		//        int16_t cnt;
		uint16_t step;
	} test; /*  */

	struct
	{
		uint16_t state; /*   */
						//        uint8_t count;
		uint16_t times;
		uint16_t value;
		int16_t offset;	   /*  */
		int16_t lastAngle; /*  */
		uint16_t code_1;   /*  */
		uint16_t code_2;   /*  */
	} memory;

	struct
	{
		struct
		{
			tSupplyCondition state;
			uint16_t voltage;
			uint16_t uvTimer;
			uint16_t ovTimer;
			uint8_t UVretryCnt;
			uint8_t OVretryCnt;
		} vs;
		struct
		{
			tTemperCondition state;
			uint16_t retryCnt;
			int16_t deg;
			uint16_t timer;
		} temp;
		struct
		{
			tIgnitionCondition state;
			uint16_t voltage;
			uint16_t uvTimer;
		} ign;
		struct
		{
			uint8_t state;
			uint8_t retryCnt;
			int16_t degree;
			uint16_t count;
		} gmr;
		uint16_t motorCurrent;
		uint8_t linError;
		uint8_t linErrRetryCnt;
		uint8_t McuFault;
		uint8_t mcuRetryCnt;
		uint8_t ObstructionRetryCnt;
		tProtectCondition protType;
		uint8_t calFault;
		uint8_t motorFault;
		uint8_t stallFault;
		uint8_t stallRetryCnt;
		uint8_t calRetryCnt;
		uint8_t motOcRetryCnt;
		uint8_t motOpenRetryCnt;
		uint8_t motShortRetryCnt;
	} diag;
} valve;

static void ValveErrorReset(void)
{
	if (valve.state == VALVE_PROTECTION)
	{

		valve.diag.motOcRetryCnt = 0;
		valve.diag.ObstructionRetryCnt = 0;
		valve.pos.retryCnt = 0;
		valve.diag.gmr.retryCnt = 0;
		valve.diag.mcuRetryCnt = 0;
		valve.diag.vs.UVretryCnt = 0;
		valve.diag.vs.OVretryCnt = 0;
		valve.diag.temp.retryCnt = 0;
	}
	if (valve.state == VALVE_FAULT)
	{
	}
}
static void ValveFaultReset(void)
{
	u16EventState = NONE_ERROR;
	valve.comm.faultMode = 0;
	MotClearFaultFlag(0);
	MotClearStallFlag(0);
	valve.pos.fault = 0;
	valve.diag.gmr.state = 0;
	valve.diag.McuFault = 0;

	valve.diag.motOcRetryCnt = 0;
	valve.diag.ObstructionRetryCnt = 0;
	valve.pos.retryCnt = 0;
	valve.diag.gmr.retryCnt = 0;
	valve.diag.mcuRetryCnt = 0;
	valve.diag.vs.UVretryCnt = 0;
	valve.diag.vs.OVretryCnt = 0;
	valve.diag.temp.retryCnt = 0;
}

static tValveState ValveInitTask(void)
{
	tValveState nextState = VALVE_INIT;
	int16_t diff = 0;

	ValveTargetAngleUpdate(valve.pos.currentAngle);
	MotSetTargetPosition(valve.pos.targetAngle);
	if ((valve.diag.ign.state == IGN_NORMAL) && (valve.elapsedTime >= 5u))
	{
		if (u16EventState == VALVE_CAL_FAULT)
		{
			valve.calibration.req2Cal = 1;
		}
		if (valve.memory.lastAngle != 0)
		{
			diff = valve.pos.currentAngle - valve.memory.lastAngle;
			if (diff < 0)
			{
				diff = -diff;
			}
			if (diff > (int16_t)C_VALVE_ACCURACY_ANGLE)
			{

				valve.calibration.req2Cal = 1;
			}
		}
		else
		{

			if (valve.pos.currentAngle <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)(45 * C_GMR_ANGLE_SCALE_FACTOR)))
			{
				diff = valve.pos.currentAngle - valve.pos.modeAngle[C_MODE_B];
			}
			else
			{
				diff = valve.pos.currentAngle - valve.pos.modeAngle[C_MODE_A];
			}
			if (diff < 0)
			{
				diff = -diff;
			}
			if (diff > (int16_t)C_VALVE_ACCURACY_ANGLE)
			{

				valve.calibration.req2Cal = 1;
			}
		}
		nextState = VALVE_STANDBY;
	}
	return nextState;
}

static tValveState ValveStandbyTask(void)
{

	tValveState nextState = VALVE_STANDBY;
	int16_t diffPos;

	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
	}

	MotRequestHardStop();

	if ((valve.calibration.req2Cal == 1) || (valve.calibration.req1Cal == 1))
	{

		nextState = VALVE_CALIBRATION;
	}
	else if (valve.comm.ForcedDiag != 0)
	{

		nextState = VALVE_DIAGRUN;
	}
	else if (valve.comm.Enable != 0)
	{

		if (valve.pos.targetAngle > valve.pos.currentAngle)
		{
			diffPos = valve.pos.targetAngle - valve.pos.currentAngle;
		}
		else
		{
			diffPos = valve.pos.currentAngle - valve.pos.targetAngle;
		}
#if POINT_TEST_ENABLE == 1
		if (valve.comm.lastMode != valve.comm.targetMode)
		{

			nextState = VALVE_READY;
		}
#else
		if ((valve.comm.lastMode != valve.comm.targetMode) && (diffPos >= (int16_t)C_VALVE_ACCURACY_ANGLE))
		{

			nextState = VALVE_READY;
		}
#endif
		valve.comm.lastMode = valve.comm.targetMode;
	}
	else
	{
	}

	return nextState;
}

static tValveState ValveReadyTask(void)
{

	tValveState nextState = VALVE_READY;

	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
		MotClearHardStop();
	}

	MotSetTargetPosition(valve.pos.targetAngle);
	nextState = VALVE_OPERATION;
	return nextState;
}

static tValveState ValveOperationTask(void)
{

	tValveState nextState = VALVE_OPERATION;
	int16_t actualPos;

	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;

		MotClearHardStop();
	}

	if (valve.elapsedTime >= valve.comm.timeOut)
	{

		valve.pos.fault = 1;
		nextState = VALVE_PROTECTION;
	}
	else
	{
#if POINT_TEST_ENABLE == 1
		if ((valve.motorMotion >= MOTION_ACC) && (valve.motorMotion <= MOTION_DEC))
		{
		}
		else
		{
			nextState = VALVE_STANDBY;
		}
#else
		if ((valve.motorMotion >= MOTION_ACC) && (valve.motorMotion <= MOTION_DEC))
		{
		}
		else
		{

			actualPos = valve.pos.currentAngle;
#if 0			
			if (valve.comm.targetMode==C_MODE_A)
			{
				if ((actualPos >= (valve.pos.modeAngle[C_MODE_A]-(int16_t)C_VALVE_ACCURACY_ANGLE))&&(actualPos <= (valve.pos.modeAngle[C_MODE_A]+(int16_t)C_VALVE_ACCURACY_ANGLE)))
				{
				}
				else
				{
					valve.pos.fault=1;

				}
			}
			else if (valve.comm.targetMode==C_MODE_B)
			{
				if ((actualPos >= (valve.pos.modeAngle[C_MODE_B]-(int16_t)C_VALVE_ACCURACY_ANGLE))&&(actualPos <= (int16_t)(valve.pos.modeAngle[C_MODE_B]+(int16_t)C_VALVE_ACCURACY_ANGLE)))
				{

				}
				else
				{
					valve.pos.fault=1;
				
				}
			}
			else {}
#else
			if ((actualPos >= (valve.pos.modeAngle[C_MODE_A] - (int16_t)C_VALVE_ACCURACY_ANGLE)) && (actualPos <= (valve.pos.modeAngle[C_MODE_A] + (int16_t)C_VALVE_ACCURACY_ANGLE)))
			{
			}
			else if ((actualPos >= (valve.pos.modeAngle[C_MODE_B] - (int16_t)C_VALVE_ACCURACY_ANGLE)) && (actualPos <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)C_VALVE_ACCURACY_ANGLE)))
			{
			}
			else
			{
				valve.pos.fault = 1;
			}
#endif
			if (valve.pos.fault != 0)
			{
				nextState = VALVE_PROTECTION;
			}
			else
			{
				nextState = VALVE_STANDBY;
			}
		}
#endif
	}

	return nextState;
}
static void calcSensorOffset(int16_t currDegree)
{
	int16_t offset = get_gmr_sensor_offset();
#if 0
	offset = (int16_t)(C_GMR_SENSOR_OFFSET*C_GMR_ANGLE_SCALE_FACTOR)-currDegree;
//	offset += (int16_t)(C_GMR_SENSOR_OFFSET*C_GMR_ANGLE_SCALE_FACTOR);
	if (offset >= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT)
	{
		offset -= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
	}
#else
	offset += (int16_t)((int16_t)(C_GMR_SENSOR_OFFSET * C_GMR_ANGLE_SCALE_FACTOR) - currDegree);
	if (offset >= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT)
	{
		offset -= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
	}
	else if (offset < 0)
	{
		offset = (-offset);
		offset = (int16_t)C_GMR_SENSOR_ANGLE_LIMIT - offset;
	}
	else
	{
	}
#endif

	set_gmr_sensor_offset(offset);
}
static tValveState ValveCalibrationTask(void)
{
	tValveState nextState = VALVE_CALIBRATION;
	int16_t diff;
	uint16_t timeOut;

	if (valve.calibration.req2Cal)
	{
		timeOut = 20000;
	}
	else
	{
		timeOut = 6000;
	}
	// uint16_t state,value;
	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
		valve.calibration.state = CALSTEP_RESET;
		valve.diag.calFault = 0;
	}
	valve.comm.lastMode = 0xff;
	switch (valve.calibration.state)
	{
	case CALSTEP_RESET:
		MotClearStallFlag(0); /* clear stall flag if set */
		MotRequestHardStop();
		valve.calibration.state = CALSTEP_START;
		break;
	case CALSTEP_START:
		MotClearHardStop();
		valve.calibration.delay = 3;
		valve.calibration.timer = 0;
		if (valve.calibration.req2Cal != 0)
		{
			ValveTargetAngleUpdate(-10 * C_GMR_ANGLE_SCALE_FACTOR); /* move to 0% position */
			MotSetTargetPosition(valve.pos.targetAngle);
			valve.calibration.state = CALSTEP_0d_POS;
		}
		else
		{
			if (valve.pos.currentAngle <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)(45 * C_GMR_ANGLE_SCALE_FACTOR)))
			{
				ValveTargetAngleUpdate(-10 * C_GMR_ANGLE_SCALE_FACTOR); /* move to 0% position */
				MotSetTargetPosition(valve.pos.targetAngle);
				valve.calibration.state = CALSTEP_0d_POS;
			}
			else
			{
				ValveTargetAngleUpdate(370 * C_GMR_ANGLE_SCALE_FACTOR); /* move to 360% position */
				MotSetTargetPosition(valve.pos.targetAngle);
				valve.calibration.state = CALSTEP_360d_POS;
			}
		}
		break;
	case CALSTEP_0d_POS:
		valve.calibration.timer += 1;
		if (valve.calibration.timer >= timeOut)
		{

			valve.calibration.state = CALSTEP_FAULT;
		}
		else if (valve.calibration.delay > 0)
		{
			valve.calibration.delay -= 1;
		}
		else
		{

			if (valve.motorMotion == MOTION_STALL)
			{
				MotClearStallFlag(0); /* clear stall flag if set */
				MotRequestHardStop();
				if (valve.calibration.offsetDone == 0)
				{
					calcSensorOffset(valve.pos.currentAngle);
					diff = valve.memory.offset - get_gmr_sensor_offset();
					if (diff < 0)
					{
						diff = -diff;
					}
					if (diff > C_VALVE_CAL_HYSTERISYS)
					{
						valve_gmr_write((uint16_t)get_gmr_sensor_offset(), (uint16_t)valve.pos.currentAngle, 0x5555);
					}
				}
				valve.calibration.offsetDone = 1;
				valve.calibration.delay = 3;
				valve.calibration.timer = 0;
				valve.calibration.state = CALSTEP_CALC;
			}
			else if (valve.motorMotion == MOTION_STOPPED)
			{

				valve.calibration.state = CALSTEP_FAULT;
			}
		}
		break;

	case CALSTEP_360d_POS:
		valve.calibration.timer += 1;
		if (valve.calibration.timer >= timeOut)
		{

			valve.calibration.state = CALSTEP_FAULT;
		}
		else if (valve.calibration.delay > 0)
		{
			valve.calibration.delay -= 1;
		}
		else
		{
			if (valve.motorMotion == MOTION_STALL)
			{
				MotClearStallFlag(0);																	/* clear stall flag if set */
				valve.calibration.d360Angle = (valve.pos.currentAngle - (int16_t)C_STOPPER_360D_ANGLE); // C_STOPPER_360D_ANGLE
				if (valve.calibration.d360Angle < 0)
				{
					valve.calibration.d360Angle += (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
				}
				else if (valve.calibration.d360Angle >= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT)
				{
					valve.calibration.d360Angle -= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
				}
				else
				{
				}
				valve.calibration.travel = valve.calibration.d360Angle - valve.calibration.d0Angle;
#if 1
				valve.pos.modeAngle[C_MODE_A] = valve.calibration.d360Angle;
#endif
				ValveTargetAngleUpdate(valve.pos.modeAngle[C_MODE_B]); /* move to init position */
				MotSetTargetPosition(valve.pos.targetAngle);
				valve.calibration.delay = 3;
				valve.calibration.state = CALSTEP_INIT_POS;
			}
			else if (valve.motorMotion == MOTION_STOPPED)
			{

				valve.calibration.state = CALSTEP_FAULT;
			}
		}
		break;
	case CALSTEP_CALC:
		if (valve.calibration.delay > 0)
		{
			valve.calibration.delay -= 1;
		}
		else
		{
			valve.calibration.d0Angle = (valve.pos.currentAngle + (int16_t)C_STOPPER_0D_ANGLE); // C_STOPPER_0D_ANGLE
			if (valve.calibration.d0Angle >= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT)
			{
				valve.calibration.d0Angle -= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
			}
			else if (valve.calibration.d0Angle < 0)
			{
				valve.calibration.d0Angle += (int16_t)C_GMR_SENSOR_ANGLE_LIMIT;
			}
			else
			{
			}
#if 1
			valve.pos.modeAngle[C_MODE_B] = valve.calibration.d0Angle;
#endif
			if (valve.calibration.req2Cal != 0)
			{
				ValveTargetAngleUpdate(370 * C_GMR_ANGLE_SCALE_FACTOR); /* move to 360% position */
				valve.calibration.state = CALSTEP_360d_POS;
			}
			else
			{
				ValveTargetAngleUpdate(valve.pos.modeAngle[C_MODE_B]); /* move to init position */
				valve.calibration.state = CALSTEP_INIT_POS;
			}
			MotClearHardStop();
			MotSetTargetPosition(valve.pos.targetAngle);
		}
		break;
	case CALSTEP_INIT_POS:
		if (valve.calibration.delay > 0)
		{
			valve.calibration.delay -= 1;
		}
		else
		{
			if (valve.motorMotion == MOTION_STALL)
			{

				MotClearStallFlag(0); /* clear stall flag if set */
				MotRequestHardStop();

				valve.calibration.state = CALSTEP_FAULT;
			}
			else if (valve.motorMotion == MOTION_STOPPED)
			{

				valve.calibration.state = CALSTEP_COMPLETED;
			}
		}
		break;

	case CALSTEP_FAULT:

		valve.calibration.req2Cal = 0;
		valve.calibration.req1Cal = 0;
		valve.diag.calFault = 1;
		u16EventState = VALVE_CAL_FAULT;
		u16EventValue = valve.pos.currentAngle;

		nextState = VALVE_FAULT;
		break;
	case CALSTEP_COMPLETED:

		valve.calibration.req2Cal = 0;
		valve.calibration.req1Cal = 0;
		nextState = VALVE_STANDBY;
		break;
	default:
		nextState = VALVE_STANDBY;
		break;
	}
	return nextState;
}
static tValveState ValveFaultTask(void)
{
	tValveState nextState = VALVE_FAULT;
	uint16_t status = 1;
	MotRequestHardStop();
	valve.comm.lastMode = 0xff;
	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
		if (valve.diag.calFault != 0)
		{
			valve.diag.calRetryCnt += 1;
		}
		if ((valve.diag.stallFault & STALL_MASK_PERMENT) != 0)
		{
			valve.diag.stallRetryCnt += 1;
		}
		if ((valve.diag.motorFault & FAULT_MASK_PHASE_A_OPEN) != 0)
		{
			valve.diag.motOpenRetryCnt += 1;
		}
		if ((valve.diag.motorFault & FAULT_MASK_PHASE_A_SHORT) != 0)
		{
			valve.diag.motShortRetryCnt += 1;
		}
	}

	if (valve.elapsedTime >= 5000u)
	{
		if (valve.diag.calRetryCnt <= 3)
		{
			valve.diag.calFault = 0;
		}
		if (valve.diag.stallRetryCnt <= 3)
		{
			MotClearStallFlag(0);
		}
		if (valve.diag.motOpenRetryCnt <= 3)
		{
			MotClearFaultFlag(1);
		}
		if (valve.diag.motShortRetryCnt <= 3)
		{
			MotClearFaultFlag(2);
		}

		if (valve.diag.calFault != 0)
		{
			status = 0;
		}
		if (MotGetStallState() != 0)
		{
			status = 0;
		}
		if (MotGetFaultState() != 0)
		{
			status = 0;
		}

		if (status != 0)
		{
			//			nextState=valve.lastState;
			nextState = VALVE_STANDBY;
		}
		else
		{
			valve.comm.faultMode = 1;
		}
	}

	return nextState;
}

static tValveState ValveProtectionTask(void)
{
	tValveState nextState = VALVE_PROTECTION;
	uint16_t status = 1;

	MotRequestHardStop();
	valve.comm.lastMode = 0xff;
	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;

		if (valve.diag.vs.state == VS_UNDERVOLTAGE)
		{
			if (valve.diag.protType != VS_LOW_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.diag.vs.UVretryCnt < 0xffu)
			{
				valve.diag.vs.UVretryCnt += 1;
			}
			valve.diag.protType = VS_LOW_ERROR;
			u16EventState = VS_LOW_ERROR;
			u16EventValue = valve.diag.vs.state;
		}
		if (valve.diag.vs.state == VS_OVERVOLTAGE)
		{
			if (valve.diag.protType != VS_HIGH_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.diag.vs.OVretryCnt < 0xffu)
			{
				valve.diag.vs.OVretryCnt += 1;
			}
			valve.diag.protType = VS_HIGH_ERROR;
			u16EventState = VS_HIGH_ERROR;
			u16EventValue = valve.diag.vs.state;
		}
		if (valve.diag.temp.state != TEMPERATURE_NORMAL)
		{
			if (valve.diag.protType != TEMP_HIGH_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.diag.temp.retryCnt < 0xffu)
			{
				valve.diag.temp.retryCnt += 1;
			}
			valve.diag.protType = TEMP_HIGH_ERROR;
			u16EventState = TEMP_HIGH_ERROR;
			u16EventValue = valve.diag.temp.deg;
		}
		if ((valve.diag.motorFault & FAULT_MASK_OVER_CURRENT) != 0)
		{
			if (valve.diag.protType != MOT_OC_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.diag.motOcRetryCnt < 0xffu)
			{
				valve.diag.motOcRetryCnt += 1;
			}
			valve.diag.protType = MOT_OC_ERROR;
		}
		if ((valve.diag.stallFault & STALL_MASK_TEMPORARY) != 0)
		{
			if (valve.diag.protType != MOT_ABSTALL_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.diag.ObstructionRetryCnt < 0xffu)
			{
				valve.diag.ObstructionRetryCnt += 1;
			}
			valve.diag.protType = MOT_ABSTALL_ERROR;
		}
		if (valve.pos.fault != 0)
		{
			if (valve.diag.protType != SENSOR_POS_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.pos.retryCnt < 0xffu)
			{
				valve.pos.retryCnt += 1;
			}
			valve.diag.protType = SENSOR_POS_ERROR;
			u16EventState = SENSOR_POS_ERROR;
			u16EventValue = valve.pos.currentAngle;
		}
		if (valve.diag.gmr.state != 0)
		{
			if (valve.diag.protType != SENSOR_OUT_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.diag.gmr.retryCnt < 0xffu)
			{
				valve.diag.gmr.retryCnt += 1;
			}
			valve.diag.protType = SENSOR_OUT_ERROR;
			u16EventState = SENSOR_OUT_ERROR;
			u16EventValue = valve.pos.currentAngle;
		}

		if (valve.diag.McuFault != 0)
		{
			if (valve.diag.protType != MCU_ERROR)
			{
				ValveErrorReset();
			}
			if (valve.diag.mcuRetryCnt < 0xffu)
			{
				valve.diag.mcuRetryCnt += 1;
			}
			valve.diag.protType = MCU_ERROR;
			u16EventState = MCU_ERROR;
			u16EventValue = valve.diag.vs.state;
		}
	}

	if (valve.elapsedTime >= 3000u)
	{
		if (valve.diag.ObstructionRetryCnt < 10)
		{
			MotClearStallFlag(1);
		}
		else
		{
			valve.comm.faultMode = 1;
		}
		if (valve.diag.motOcRetryCnt < 10)
		{
			MotClearFaultFlag(3);
		}
		else
		{
			valve.comm.faultMode = 1;
		}
		if (valve.pos.retryCnt < 10)
		{
			valve.pos.fault = 0;
		}
		else
		{
			valve.comm.faultMode = 1;
		}
		if (valve.diag.gmr.retryCnt < 10)
		{
			valve.diag.gmr.state = 0;
		}
		else
		{
			valve.comm.faultMode = 1;
		}
	}

	if (valve.diag.vs.state != VS_NORMAL)
	{
		status = 0;
	}
	else
	{
		if (valve.diag.vs.UVretryCnt >= 30)
		{
			status = 0;
			valve.comm.faultMode = 1;
		}
		if (valve.diag.vs.OVretryCnt >= 30)
		{
			status = 0;
			valve.comm.faultMode = 1;
		}
	}
	if (valve.diag.temp.state != TEMPERATURE_NORMAL)
	{
		status = 0;
	}
	else
	{
		if (valve.diag.temp.retryCnt >= 10)
		{
			status = 0;
			valve.comm.faultMode = 1;
		}
	}
	if (valve.diag.McuFault != 0)
	{
		status = 0;
	}
	else
	{
		if (valve.diag.mcuRetryCnt >= 10)
		{
			status = 0;
			valve.comm.faultMode = 1;
		}
	}
	if ((valve.diag.stallFault & STALL_MASK_TEMPORARY) != 0)
	{
		status = 0;
	}
	if ((valve.diag.motorFault & FAULT_MASK_OVER_CURRENT) != 0)
	{
		status = 0;
	}

	if ((valve.diag.gmr.state != 0) || (valve.pos.fault != 0))
	{
		status = 0;
	}
	if (valve.linLiveTimeOut == 0)
	{
		status = 0;
	}
	if ((status != 0) && (valve.comm.faultMode == 0))
	{
		//		nextState=valve.lastState;
		nextState = VALVE_STANDBY;
	}

	return nextState;
}
static tValveState ValveLowPowerTask(void)
{
	tValveState nextState = VALVE_LOWPOWER;

	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
	}
	if (valve.diag.ign.state != IGN_OFF)
	{

		nextState = VALVE_STANDBY;
	}

	if (valve.elapsedTime >= 60000u) /*60sec*/
	{
		AppLinSleepEnter();
	}
	return nextState;
}
static tValveState ValvePowerLatchTask(void)
{
	tValveState nextState = VALVE_POWERLATCH;

	MotRequestHardStop();
	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
	}
	if (valve.diag.ign.state != IGN_OFF)
	{

		nextState = valve.lastState;
	}
	else
	{
	}
	return nextState;
}

static tValveState ValveDiagRunTask(void)
{

	tValveState nextState = VALVE_DIAGRUN;

	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
		valve.test.step = 0;
		MotClearHardStop();
	}
	valve.comm.lastMode = 0xff;
	if (valve.comm.ForcedDiag == 0)
	{
		if (valve.comm.moving == 0)
		{
			nextState = VALVE_STANDBY;
		}
	}
	else
	{
		/* B->A->B*/
		if ((valve.motorMotion >= MOTION_ACC) && (valve.motorMotion <= MOTION_DEC))
		{
			valve.elapsedTime = 0;
		}
		else
		{

			if (valve.elapsedTime >= 2000u)
			{
				if (valve.test.step == 0)
				{
					valve.test.step = 1;
					ValveTargetAngleUpdate(valve.pos.modeAngle[C_MODE_B]);
					MotSetTargetPosition(valve.pos.targetAngle);
				}
				else if (valve.test.step == 1)
				{
#if 0
					valve.test.step=0;
#else
					valve.test.step = 2;
#endif
					ValveTargetAngleUpdate(valve.pos.modeAngle[C_MODE_A]);
					MotSetTargetPosition(valve.pos.targetAngle);
				}
				else if (valve.test.step == 2) /*20250714*/
				{
					valve.test.step = 3;
					ValveTargetAngleUpdate(valve.pos.modeAngle[C_MODE_B]);
					MotSetTargetPosition(valve.pos.targetAngle);
				}
				else
				{
				}
			}
		}
	}

	return nextState;
}

static tValveState ValveUndefTask(void)
{

	tValveState nextState = VALVE_UNDEF;

	if (valve.initStatus != 0)
	{
		valve.initStatus = 0;
	}
	if (valve.elapsedTime >= 1000u)
	{
	}
	return nextState;
}
static uint16_t check_fault_mode(void)
{
	uint16_t status = 0;
	if (valve.motorMotion == MOTION_FAULT)
	{

		if ((valve.diag.motorFault & FAULT_MASK_PHASE_A_OPEN) != 0)
		{
			status = 1;
		}
		else if ((valve.diag.motorFault & FAULT_MASK_PHASE_A_SHORT) != 0)
		{
			status = 1;
		}
		else
		{
			status = 0;
		}
	}
	else if (valve.motorMotion == MOTION_STALL)
	{

		if (valve.state != VALVE_CALIBRATION)
		{
			if ((valve.diag.stallFault & STALL_MASK_PERMENT) != 0)
			{
				status = 1;
			}
			else
			{
				status = 0;
			}
		}
		else
		{
			status = 0;
		}
	}
	else
	{
		status = 0;
	}
	return status;
}
static uint16_t check_protect_mode(void)
{
	uint16_t status = 0;
	if ((valve.diag.vs.state == VS_UNDERVOLTAGE) || (valve.diag.vs.state == VS_OVERVOLTAGE) || (valve.diag.temp.state == TEMPERATURE_HIGH))
	{
		status = 1;
	}
	if ((valve.diag.stallFault & STALL_MASK_TEMPORARY) != 0)
	{
		status = 1;
	}
	if ((valve.diag.motorFault & FAULT_MASK_OVER_CURRENT) != 0)
	{
		status = 1;
	}
	if (valve.diag.McuFault != 0)
	{
		status = 1;
	}
#if 1
	if ((valve.diag.gmr.state != 0) || (valve.pos.fault != 0))
	{
		status = 1;
	}
#endif
	if (valve.linLiveTimeOut == 0)
	{
		status = 1;
	}
	return status;
}
static void valveDiagVs(void)
{
	valve.diag.vs.voltage = get_conv_supply_voltage();
	switch (valve.diag.vs.state)
	{
	case VS_NORMAL:
		if (valve.diag.vs.voltage <= VS_UNDER_STOP)
		{
			valve.diag.vs.uvTimer++;
			if (valve.diag.vs.uvTimer >= VS_ENTER_COUNT)
			{
				valve.diag.vs.uvTimer = 0u;
				valve.diag.vs.state = VS_UNDERVOLTAGE;
			}
		}
		else if (valve.diag.vs.voltage >= VS_OVER_STOP)
		{
			valve.diag.vs.ovTimer++;
			if (valve.diag.vs.ovTimer >= VS_ENTER_COUNT)
			{
				valve.diag.vs.ovTimer = 0u;
				valve.diag.vs.state = VS_OVERVOLTAGE;
			}
		}
		else
		{
#if 1
			if (valve.diag.vs.uvTimer > 0)
			{
				valve.diag.vs.uvTimer--;
			}
			if (valve.diag.vs.ovTimer > 0)
			{
				valve.diag.vs.ovTimer--;
			}
#else
			valve.diag.vs.uvTimer = 0u;
			valve.diag.vs.ovTimer = 0;
#endif
		}
		break;
	case VS_UNDERVOLTAGE:

		if (valve.diag.vs.voltage >= VS_UNDER_RETURN)
		{
			valve.diag.vs.uvTimer++;
			if (valve.diag.vs.uvTimer >= VS_ENTER_COUNT)
			{
				valve.diag.vs.uvTimer = 0u;
				valve.diag.vs.state = VS_NORMAL;
			}
		}
		else
		{
#if 1
			if (valve.diag.vs.uvTimer > 0)
			{
				valve.diag.vs.uvTimer--;
			}
#else
			valve.diag.vs.uvTimer = 0u;
#endif
		}
		break;
	case VS_OVERVOLTAGE:

		if (valve.diag.vs.voltage <= VS_OVER_RETURN)
		{
			valve.diag.vs.ovTimer++;
			if (valve.diag.vs.ovTimer >= VS_ENTER_COUNT)
			{
				valve.diag.vs.ovTimer = 0u;
				valve.diag.vs.state = VS_NORMAL;
			}
		}
		else
		{
#if 1
			if (valve.diag.vs.ovTimer > 0)
			{
				valve.diag.vs.ovTimer--;
			}
#else
			valve.diag.vs.ovTimer = 0u;
#endif
		}
		break;

	default: /* VS_INIT */
#if 0			
			if(valve.diag.vs.voltage < VS_UNDER_STOP)
			{
				valve.diag.vs.uvTimer++;
				if(valve.diag.vs.uvTimer >= VS_ENTER_COUNT)
				{
					valve.diag.vs.uvTimer = 0u;
					valve.diag.vs.state = VS_UNDERVOLTAGE;

				}        

			}
			else if(valve.diag.vs.voltage > VS_OVER_STOP)
			{
				valve.diag.vs.ovTimer++;
				if(valve.diag.vs.ovTimer >= VS_ENTER_COUNT)
				{
					valve.diag.vs.ovTimer = 0u;
					valve.diag.vs.state = VS_OVERVOLTAGE;

				}
			}
			else
#endif
	{
		valve.diag.vs.uvTimer = 0u;
		valve.diag.vs.ovTimer = 0u;
		valve.diag.vs.state = VS_NORMAL;
	}
	break;
	}
}
static void valveDiagTemp(void)
{
	valve.diag.temp.deg = get_conv_ic_temperature();

	switch (valve.diag.temp.state)
	{
	case TEMPERATURE_NORMAL:
		if (valve.diag.temp.deg > (int16_t)TEMP_OVER_STOP)
		{
			valve.diag.temp.timer++;
			if (valve.diag.temp.timer >= TEMP_ENTER_COUNT)
			{
				valve.diag.temp.timer = 0u;
				valve.diag.temp.state = TEMPERATURE_HIGH;
			}
		}
		else
		{
#if 1
			if (valve.diag.temp.timer > 0)
			{
				valve.diag.temp.timer--;
			}
#else
			valve.diag.temp.timer = 0u;
#endif
		}
		break;
	case TEMPERATURE_LOW:

		if (valve.diag.temp.deg >= (int16_t)TEMP_UNDER_RETURN)
		{
			valve.diag.temp.timer++;
			if (valve.diag.temp.timer >= TEMP_ENTER_COUNT)
			{
				valve.diag.temp.timer = 0u;
				valve.diag.temp.state = TEMPERATURE_NORMAL;
			}
		}
		else
		{
#if 1
			if (valve.diag.temp.timer > 0)
			{
				valve.diag.temp.timer--;
			}
#else
			valve.diag.temp.timer = 0u;
#endif
		}
		break;
	case TEMPERATURE_HIGH:

		if (valve.diag.temp.deg <= (int16_t)TEMP_OVER_RETURN)
		{
			valve.diag.temp.timer++;
			if (valve.diag.temp.timer >= TEMP_ENTER_COUNT)
			{
				valve.diag.temp.timer = 0u;
				valve.diag.temp.state = TEMPERATURE_NORMAL;
			}
		}
		else
		{
#if 1
			if (valve.diag.temp.timer > 0)
			{
				valve.diag.temp.timer--;
			}
#else
			valve.diag.temp.timer = 0u;
#endif
		}
		break;
	default:
#if 0			
			if(valve.diag.temp.deg > (int16_t)TEMP_OVER_RETURN)
			{
				valve.diag.temp.timer++;
				if(valve.diag.temp.timer >= TEMP_ENTER_COUNT)
				{
					valve.diag.temp.timer = 0u;
					valve.diag.temp.state = TEMPERATURE_HIGH;

				}        

			}
			else
#endif
	{
		valve.diag.temp.timer = 0u;
		valve.diag.temp.state = TEMPERATURE_NORMAL;
	}
	break;
	}
}
static void valveDiagIgn(void)
{
	static uint16_t ignNormalTimer = 0;
	valve.diag.ign.voltage = get_conv_ignition_voltage();

	switch (valve.diag.ign.state)
	{
	case IGN_NORMAL:
		if (valve.diag.ign.voltage <= IGN_UNDER_STOP)
		{
			valve.diag.ign.uvTimer++;
			if (valve.diag.ign.uvTimer >= IGN_ENTER_COUNT)
			{
				valve.diag.ign.uvTimer = 0u;
				valve.diag.ign.state = IGN_OFF;
			}
		}
		else
		{
#if 1
			if (valve.diag.ign.uvTimer > 0)
			{
				valve.diag.ign.uvTimer--;
			}
#else
			valve.diag.ign.uvTimer = 0u;
#endif
		}
		break;

	case IGN_OFF:

		if (valve.diag.ign.voltage >= IGN_UNDER_RETURN)
		{
			valve.diag.ign.uvTimer++;
			if (valve.diag.ign.uvTimer >= IGN_ENTER_COUNT)
			{
				valve.diag.ign.uvTimer = 0u;
				valve.diag.ign.state = IGN_NORMAL;
			}
		}
		else
		{
#if 1
			if (valve.diag.ign.uvTimer > 0)
			{
				valve.diag.ign.uvTimer--;
			}
#else
			valve.diag.ign.uvTimer = 0u;
#endif
		}
		break;

	default: /* VS_INIT */
		if (valve.diag.ign.voltage > IGN_UNDER_STOP)
		{
			ignNormalTimer += 1;
			if (ignNormalTimer >= 10u)
			{
				valve.diag.ign.state = IGN_NORMAL;
			}
			valve.diag.ign.uvTimer = 0u;
		}
		else if (valve.diag.ign.voltage < IGN_UNDER_STOP)
		{
			valve.diag.ign.voltage = 0;
			valve.diag.ign.uvTimer++;
			if (valve.diag.ign.uvTimer >= 10u)
			{
				valve.diag.ign.uvTimer = 0u;
				valve.diag.ign.state = IGN_OFF;
			}
		}
		else
		{
			valve.diag.ign.uvTimer = 0u;
			ignNormalTimer = 0;
		}
		break;
	}
}
static void valveDiagSensor(void)
{
	uint8_t sensor_f = SensorGetState();
	valve.diag.motorCurrent = get_conv_mot_current();

	if ((valve.state != VALVE_CALIBRATION) && (valve.motorMotion == MOTION_RUNNING))
	{

		if ((valve.diag.motorCurrent >= 500) && (sensor_f == C_STATUS_STOP))
		{

			valve.diag.gmr.count += 1;
		}
		else
		{
			if (valve.diag.gmr.count > 0)
				valve.diag.gmr.count -= 1;
		}

		if (valve.diag.gmr.count > 2000) /*20250715*/
		{
			valve.diag.gmr.count = 0;
			valve.diag.gmr.state = 1;
		}
	}
	else
	{
		valve.diag.gmr.count = 0;
	}
}
static void valveDiagMcu(void)
{
	static uint16_t LV_filterCnt = 0, HV_filterCnt = 0;
	uint16_t voltage = get_conv_vdda_voltage();
	if (valve.diag.McuFault == 0)
	{
		HV_filterCnt = 0;
		if (voltage <= 300) /*scale: 10mV*/
		{
			LV_filterCnt += 1;
			if (LV_filterCnt >= 500)
			{
				valve.diag.McuFault = 1;
			}
		}
		else
		{
			if (LV_filterCnt > 0)
				LV_filterCnt -= 1;
		}
	}
	else
	{
		LV_filterCnt = 0;
		if (voltage >= 320) /*scale: 10mV*/
		{
			HV_filterCnt += 1;
			if (HV_filterCnt >= 500)
			{
				valve.diag.McuFault = 0;
			}
		}
		else
		{
			if (HV_filterCnt > 0)
				HV_filterCnt -= 1;
		}
	}
}
/**
 * \brief Sleep Control Task
 *
 */

static void ValvePowerOffTask(void)
{
	int16_t cOffset = 0, cPos = 0, diff = 0;

	if (valve.diag.ign.state == IGN_OFF)
	{
		if (valve.ignOffCnt < 0xffffu)
		{
			valve.ignOffCnt += 1;
		}

		if (valve.ignOffCnt >= 60000u) /*60sec*/
		{
			if (valve.sleepState == 0)
			{
				/*sleep ready -> calibration 수행 (진단사양 없을시 quick cal, 아니면 full cal */
				if ((check_fault_mode() == 0) && (check_protect_mode() == 0))
				{
					ValveFaultReset();
					valve.calibration.req1Cal = 1;
				}
				else
				{
					ValveFaultReset();
					valve.calibration.req2Cal = 1;
				}
				valve.sleepState = 1;
			}
			else
			{
				if (valve.state != VALVE_CALIBRATION)
				{
					if (valve.sleepState == 1)
					{
						/*이벤트 이력저장*/
						cOffset = get_gmr_sensor_offset();
						cPos = valve.pos.currentAngle;
						diff = cPos - valve.memory.lastAngle;
						if (diff < 0)
						{
							diff = -diff;
						}
						if ((cOffset != valve.memory.offset) || (diff > (int16_t)C_VALVE_ACCURACY_ANGLE))
						{
							valve_gmr_write((uint16_t)cOffset, (uint16_t)cPos, 0x5555);
						}
						if ((u16EventState != valve.memory.state) || (u16EventValue != valve.memory.value))
						{
							valve_diag_write(u16EventState, u16EventValue, 0xAAAA);
						}
						valve.sleepState = 2;
					}
					else
					{
						MotRequestHardStop();
						AppLinSleepEnter();
					}
				}
			}
		}
	}
	else
	{
		valve.ignOffCnt = 0;
		valve.sleepState = 0;
	}
}

static void calc_PosToLinData(int16_t currentAngle)
{
// #define CAL_POS_ANGLE_THD	(50* C_GMR_ANGLE_SCALE_FACTOR)
#define CAL_POS_ANGLE_THD (1 * C_GMR_ANGLE_SCALE_FACTOR)
	switch (valve.comm.actualMode)
	{
	case C_MODE_A:
		if (currentAngle <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)CAL_POS_ANGLE_THD))
		{
			valve.comm.actualMode = C_MODE_B;
		}
		else
		{
		}
		break;
	case C_MODE_B:
		if (currentAngle >= (valve.pos.modeAngle[C_MODE_A] - (int16_t)CAL_POS_ANGLE_THD))
		{
			valve.comm.actualMode = C_MODE_A;
		}
		else
		{
		}
		break;
	default:
		valve.comm.actualMode = C_MODE_B;
		break;
	}
}
void ValveLinGetCommand(void)
{
	int16_t pos;

	valve.comm.Enable = l_bool_rd_Fwv_MoveEnable();

	if (l_bool_rd_Fwv_Initial() != valve.comm.Initial)
	{
		valve.comm.Initial = l_bool_rd_Fwv_Initial();
		if (valve.comm.Initial)
		{
			valve.calibration.req2Cal = 1;
			ValveFaultReset();
		}
	}

	valve.comm.ForcedDiag = l_bool_rd_Fwv_ForcedDiag();

	if ((valve.comm.ForcedDiag == 0) && (valve.comm.Enable != 0))
	{

		valve.comm.targetMode = l_u8_rd_Fwv_Target_Mode();
		if (valve.comm.targetMode == C_MODE_A)
		{
			pos = valve.pos.modeAngle[C_MODE_A];
			ValveTargetAngleUpdate(pos);
		}
		else if (valve.comm.targetMode == C_MODE_B)
		{
			pos = valve.pos.modeAngle[C_MODE_B];
			ValveTargetAngleUpdate(pos);
		}
		else
		{
#if POINT_TEST_ENABLE == 1
			valve.comm.targetMode = l_u8_rd_Fwv_Reserved1();
			if (valve.comm.targetMode > 0)
			{
				pos = (C_GMR_TARGET_OFFSET * C_GMR_ANGLE_SCALE_FACTOR) + (valve.comm.targetMode * C_GMR_ANGLE_SCALE_FACTOR);
				ValveTargetAngleUpdate(pos);
			}
			else
			{
				valve.comm.targetMode = 0xFFu;
			}

#endif
		}

#if SOFTSTART_TEST_ENABLE == 1
		uint8_t accVal = l_u8_rd_Fwv_Reserved1();
		switch (accVal)
		{
		case 1:
			MotSetSoftStartAcc(16);
			break;
		case 2:
			MotSetSoftStartAcc(8);
			break;
		case 3:
			MotSetSoftStartAcc(6);
			break;
		case 4:
			MotSetSoftStartAcc(4);
			break;
		case 5:
			MotSetSoftStartAcc(3);
			break;
		default:
			MotSetSoftStartAcc(30);
			break;
		}

#endif
#if DUTY_ADJUST_ENABLE == 1
		uint8_t dutyVal = l_u8_rd_Fwv_Reserved1();
		switch (dutyVal)
		{
		case 1:
			MotSetMaxDuty(C_MOT_MAXDUTY_SET * 0.5f);
			break;
		case 2:
			MotSetMaxDuty(C_MOT_MAXDUTY_SET * 0.6f);
			break;
		case 3:
			MotSetMaxDuty(C_MOT_MAXDUTY_SET * 0.7f);
			break;
		case 4:
			MotSetMaxDuty(C_MOT_MAXDUTY_SET * 0.8f);
			break;
		case 5:
			MotSetMaxDuty(C_MOT_MAXDUTY_SET * 0.9f);
			break;
		default:
			MotSetMaxDuty(C_MOT_MAXDUTY_SET);
			break;
		}
#endif
	}
	else
	{
	}
}
void ValveLinUpdateSignals(void) /*20250714*/
{
	/* Byte 0 */
	l_u8_wr_Fwv_Actual_Mode(valve.comm.actualMode);

	l_bool_wr_Fwv_Position_Fault(valve.pos.fault);
	//	if (valve.state==VALVE_FAULT)
	if (valve.comm.faultMode != 0) /*20250714*/
	{
		l_bool_wr_Fwv_FaultMode(1);
	}
	else
	{
		l_bool_wr_Fwv_FaultMode(0);
	}
	if (valve.state == VALVE_PROTECTION)
	{
		if (valve.comm.faultMode == 0) /*20250714*/
		{
			l_bool_wr_Fwv_ProtectMode(1);
		}
		else
		{
			l_bool_wr_Fwv_ProtectMode(0);
		}
	}
	else
	{
		l_bool_wr_Fwv_ProtectMode(0);
	}
	if (valve.state == VALVE_CALIBRATION)
	{
		l_bool_wr_Fwv_InitialSta(1);
	}
	else
	{
		l_bool_wr_Fwv_InitialSta(0);
	}
	if (valve.comm.faultMode != 0) /*20250714*/
	{
		if (valve.diag.calFault != 0)
		{
			l_bool_wr_Fwv_Calibration_Fail(1);
		}
		else
		{
			l_bool_wr_Fwv_Calibration_Fail(0);
		}
	}
	else
	{
		l_bool_wr_Fwv_Calibration_Fail(0);
	}

	/* Byte 1 */
	l_bool_wr_Fwv_MoveEnable_Status(valve.comm.moving);

	if (valve.motorMotion == MOTION_FAULT) /*20250714*/
	{
		if ((valve.diag.stallFault != 0) && (valve.comm.faultMode != 0))
		{
			l_bool_wr_Fwv_Motor_Stall(1);
		}
		else
		{
			l_bool_wr_Fwv_Motor_Stall(0);
		}
	}
	else
	{
		if ((valve.diag.stallFault & STALL_MASK_TEMPORARY) != 0)
		{
			l_bool_wr_Fwv_Motor_Stall(1);
		}
		else
		{
			l_bool_wr_Fwv_Motor_Stall(0);
		}
	}

	if (((valve.diag.motorFault & FAULT_MASK_PHASE_A_OPEN) != 0) && (valve.comm.faultMode != 0)) /*20250714*/
	{
		l_bool_wr_Fwv_Open_Circuit(1);
	}
	else
	{
		l_bool_wr_Fwv_Open_Circuit(0);
	}
	if (((valve.diag.motorFault & FAULT_MASK_PHASE_A_SHORT) != 0) && (valve.comm.faultMode != 0)) /*20250714*/
	{
		l_bool_wr_Fwv_Short_Circuit(1);
	}
	else
	{
		l_bool_wr_Fwv_Short_Circuit(0);
	}

	if (valve.diag.vs.state == VS_NORMAL)
	{
		l_bool_wr_Fwv_Undervoltage(0);
		l_bool_wr_Fwv_Overvoltage(0);
	}
	else if (valve.diag.vs.state == VS_UNDERVOLTAGE)
	{
		l_bool_wr_Fwv_Undervoltage(1);
		l_bool_wr_Fwv_Overvoltage(0);
	}
	else if (valve.diag.vs.state == VS_OVERVOLTAGE)
	{
		l_bool_wr_Fwv_Undervoltage(0);
		l_bool_wr_Fwv_Overvoltage(1);
	}
	else
	{
	}

	if ((valve.diag.motorFault & FAULT_MASK_OVER_CURRENT) != 0)
	{
		l_bool_wr_Fwv_Overcurrent(1);
	}
	else
	{
		l_bool_wr_Fwv_Overcurrent(0);
	}

	if (valve.diag.temp.state == TEMPERATURE_HIGH)
	{
		l_bool_wr_Fwv_Overtemperature(1);
	}
	else
	{
		l_bool_wr_Fwv_Overtemperature(0);
	}

	/* Byte 2 [2..0] */
	if (valve.state == VALVE_DIAGRUN)
	{
		l_bool_wr_Fwv_Diag_Forced_Status(1);
	}
	else
	{
		l_bool_wr_Fwv_Diag_Forced_Status(0);
	}
	l_bool_wr_Fwv_Position_Sensor_Fault(valve.diag.gmr.state);
	l_bool_wr_Fwv_CommErr(valve.diag.linError);
	valve.diag.linError = 0;
	/* Byte 2 [7..3] and Byte 3 ~ 4*/
	l_u16_wr_Fwv_SW_Version(SW_VERSION);
#if 1
	int16_t pos = valve.pos.currentAngle;
	if (pos >= (valve.pos.modeAngle[C_MODE_A] - (int16_t)C_VALVE_ACCURACY_ANGLE))
	{
		l_u8_wr_Fwv_Stall_State(0);
	}
	else if (pos <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)C_VALVE_ACCURACY_ANGLE))
	{
		l_u8_wr_Fwv_Stall_State(0);
	}
	else
	{
		l_u8_wr_Fwv_Stall_State(1);
	}
#else
	int16_t pos = valve.pos.currentAngle;
	if (pos <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)(22.5f * C_GMR_ANGLE_SCALE_FACTOR)))
	{

		l_u8_wr_Fwv_Stall_State(0);
	}
	else if ((pos >= (valve.pos.modeAngle[C_MODE_B] + (int16_t)(22.5f * C_GMR_ANGLE_SCALE_FACTOR))) && (pos <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)(45 * C_GMR_ANGLE_SCALE_FACTOR))))
	{

		l_u8_wr_Fwv_Stall_State(1);
	}
	else if ((pos >= (valve.pos.modeAngle[C_MODE_B] + (int16_t)(45 * C_GMR_ANGLE_SCALE_FACTOR))) && (pos <= (valve.pos.modeAngle[C_MODE_B] + (int16_t)(67.5f * C_GMR_ANGLE_SCALE_FACTOR))))
	{

		l_u8_wr_Fwv_Stall_State(2);
	}
	else
	{

		l_u8_wr_Fwv_Stall_State(3);
	}

#endif
}
void ValveTargetAngleUpdate(int16_t angle)
{
	valve.pos.targetAngle = angle;
}
tValveState get_sys_valve_mode(void)
{
	tValveState status = valve.state;
	if (valve.diag.ign.state == IGN_OFF)
	{
		status = VALVE_POWERLATCH;
	}
	return status;
}
tValveState get_valve_mode(void)
{
	return valve.state;
}
uint16_t get_valve_voltage(void)
{
	return valve.diag.vs.voltage;
}
int16_t get_valve_temperature(void)
{
	return valve.diag.temp.deg;
}
uint16_t get_valve_motCurrent(void)
{
	return valve.diag.motorCurrent;
}

void AppValveInit(void)
{

	valve.state = VALVE_INIT;
	valve.lastState = VALVE_INIT;
	valve.elapsedTime = 0;
	valve.sleepState = 0;
	valve.initStatus = 0;
	valve.linLiveTimeOut = 4000;
	valve.pos.currentAngle = 0;
	valve.pos.targetAngle = 0;
	valve.pos.modeAngle[C_MODE_A] = (int16_t)C_VALVE_MODE_A_ANGLE;
	valve.pos.modeAngle[C_MODE_B] = (int16_t)C_VALVE_MODE_B_ANGLE;
	valve.pos.fault = 0;
	valve.pos.retryCnt = 0;

	valve.calibration.req2Cal = 0;
	valve.calibration.req1Cal = 0;
	valve.calibration.offsetDone = 0;

	valve.comm.Initial = 0;
	valve.comm.Enable = 0;
	valve.comm.ForcedDiag = 0;
	valve.comm.targetMode = 0xFFu;
	valve.comm.lastMode = 0xFFu;
	valve.comm.actualMode = C_MODE_B;
	valve.comm.moving = 0;
	valve.comm.timeOut = 5000;
	valve.comm.faultMode = 0;

	valve.diag.protType = NONE_ERROR;
	valve.diag.calFault = 0;
	valve.diag.motorFault = 0;
	valve.diag.stallFault = 0;
	valve.diag.McuFault = 0;
	valve.diag.mcuRetryCnt = 0;
	valve.diag.vs.state = VS_UNDEF;
	valve.diag.vs.UVretryCnt = 0;
	valve.diag.vs.OVretryCnt = 0;
	valve.diag.ign.state = IGN_UNDEF;
	valve.diag.ign.uvTimer = 0;
	valve.diag.temp.state = TEMPERATURE_UNDEF;
	valve.diag.temp.retryCnt = 0;
	valve.diag.gmr.state = 0;
	valve.diag.gmr.retryCnt = 0;
	valve.diag.stallRetryCnt = 0;
	valve.diag.calRetryCnt = 0;
	valve.diag.ObstructionRetryCnt = 0;
	valve.diag.motOcRetryCnt = 0;
	valve.diag.motOpenRetryCnt = 0;
	valve.diag.motShortRetryCnt = 0;
	valve.diag.linError = 0;
	valve.diag.linErrRetryCnt = 0;
	if (eeprom_ReadValveConfig(&valve_gmr_data))
	{
		valve.memory.offset = (int16_t)valve_gmr_data.E1DATA0; // 250709-2 - EEPROM Load 1st -> Global Variables
		if ((valve.memory.offset > 0) && (valve.memory.offset <= (int16_t)C_GMR_SENSOR_ANGLE_LIMIT))
		{

			set_gmr_sensor_offset(valve.memory.offset);
		}
		valve.memory.lastAngle = (int16_t)valve_gmr_data.E1DATA1; // 250709-2 - EEPROM Load 2nd -> Global Variables
		valve.memory.code_1 = valve_gmr_data.E1DATA2;			  // 250709-2 - EEPROM Load 3rd -> Global Variables
	}
	else
	{
	}
	if (eeprom_ReadDiagConfig(&valve_diag_data))
	{
		valve.memory.state = valve_diag_data.E1DATA0;
		valve.memory.value = valve_diag_data.E1DATA1;
		valve.memory.code_2 = valve_diag_data.E1DATA2;
		u16EventState = valve.memory.state;
		u16EventValue = valve.memory.value;
	}
	else
	{
	}
}
/**
 * \brief Actuator task called by every 1ms
 *
 */

void AppValveTask(void)
{
	tValveState nextState = valve.state;
	uint16_t protect_mode = 0, fault_err = 0;

#if 0
#if LIN_DEBUG_ENABLE
			g_u16DebugData[2]=valve.pos.targetAngle;
			g_u16DebugData[3]=valve.pos.currentAngle;
			g_u16DebugData[4]=get_gmr_sensor_offset();
			g_u16DebugData[5]=valve.diag.ign.state;
			g_u16DebugData[6]=	valve.memory.state;
			g_u16DebugData[7]=valve.memory.value;

			g_u16DebugData[10]=valve.state;
			g_u16DebugData[11]=valve.motorMotion;
#endif
#endif

	valveDiagVs();
	valveDiagTemp();
	valveDiagIgn();

	ValvePowerOffTask();
	valve.motorMotion = MotGetState();
	valve.diag.motorFault = MotGetFaultState();
	valve.diag.stallFault = MotGetStallState();
	valveDiagSensor();
	valveDiagMcu();

	fault_err = check_fault_mode();
	protect_mode = check_protect_mode();
	if ((valve.state != VALVE_FAULT) && (fault_err != 0))
	{

		valve.initStatus = 1;
		valve.elapsedTime = 0;
		valve.lastState = valve.state;
		valve.state = VALVE_FAULT;
	}
	else if ((valve.state != VALVE_FAULT) && (valve.state != VALVE_PROTECTION) && (protect_mode != 0))
	{
		valve.initStatus = 1;
		valve.elapsedTime = 0;
		valve.lastState = valve.state;
		valve.state = VALVE_PROTECTION;
	}
	else
	{
	}

	valve.pos.currentAngle = MotGetCurrentPosition();
	calc_PosToLinData(valve.pos.currentAngle);
	if ((valve.motorMotion >= MOTION_ACC) && (valve.motorMotion <= MOTION_DEC))
	{
		valve.comm.moving = 1;
	}
	else
	{
		valve.comm.moving = 0;
	}
	switch (valve.state)
	{
	case VALVE_INIT:
		nextState = ValveInitTask();
		break;
	case VALVE_STANDBY:
		nextState = ValveStandbyTask();
		break;
	case VALVE_READY:
		nextState = ValveReadyTask();
		break;
	case VALVE_OPERATION:
		nextState = ValveOperationTask();
		break;
	case VALVE_DIAGRUN:
		nextState = ValveDiagRunTask();
		break;
	case VALVE_CALIBRATION:
		nextState = ValveCalibrationTask();
		break;
	case VALVE_FAULT:
		nextState = ValveFaultTask();
		break;
	case VALVE_PROTECTION:
		nextState = ValveProtectionTask();
		break;
	case VALVE_POWERLATCH:
		nextState = ValvePowerLatchTask();
		break;
	case VALVE_LOWPOWER:
		nextState = ValveLowPowerTask();
		break;
	case VALVE_UNDEF:
		nextState = ValveUndefTask();
		break;
	default:
		nextState = VALVE_STANDBY;
		break;
	}

	/* state changed */
	if (valve.state != nextState)
	{

		valve.initStatus = 1;
		valve.elapsedTime = 0;
		if (valve.state != VALVE_LOWPOWER)
		{
			valve.lastState = valve.state;
		}
		valve.state = nextState;
	}
	else
	{
		if (valve.elapsedTime < 0xffffu)
			valve.elapsedTime += 1;
	}

	if (valve.linLiveTimeOut > 0)
	{
		valve.linLiveTimeOut -= 1;
	}
	else
	{
		valve.diag.linError = 1;
	}
	//	valve.comm.actualMode
	if (Fwv_Request_Event != 0)
	{
		Fwv_Request_Event = 0;
		//		ValveLinGetCommand();
		valve.linLiveTimeOut = 4000;
	}
	if (Fwv_Response_Event != 0)
	{
		Fwv_Response_Event = 0;
		//		ValveLinUpdateSignals();
		valve.linLiveTimeOut = 4000;
	}
}
