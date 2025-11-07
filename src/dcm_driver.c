/*
 * dcm_driver.c
 *
 *  Created on: 2025. 5. 20.
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
/* application */
#include "defines.h"
#include "dcm_driver.h"
#include "app_sensor.h"
#include "diagnostic.h"
#include "pwm.h"
#include "protection.h"
#include "AppValve.h"
#include "AppLin.h"
#include "eeprom_app.h"
/* local variables */
struct {
    tMotState state;
    tMotState lastState;	
	uint16_t initStatus;		
	uint16_t elapsedTime;		
    uint16_t requestStop;        /* 0: No request, 1: Request stop */
    int16_t speed;              /* current speed [PPS] */
    tMotDirection direction;          /* 1 : forward, -1 : backward */
    tMotDirection lastDirection; 	
    uint16_t runTimeOut;        /*  */	
	uint16_t holdTime;
    struct {
	int16_t current;
	int16_t target;
	int16_t lastTarget;		
	int16_t Delta;
	uint8_t newTarget;			
	uint8_t posReached;		
    } pos;

    struct {
		uint16_t enable;		
        uint16_t outThreshold;           /*  */
        uint16_t accDuty;           /* acceleration duty */
    } softStart;
    struct {
	uint8_t enable;	
	uint8_t completed;		
        uint16_t inThreshold;           /*  */
        uint16_t dccDuty;           /* dcceleration duty */
    } softStop;

    struct {
        uint16_t enable;             /*  */
        uint16_t duty;              /* */
        uint16_t maxDuty;              /* */			
        uint16_t minDuty;              /* */		
    } out;

    struct {
        int16_t rawCurrent;  /* motor current [mA] */
        int16_t filteredCurrent;   /* filtered motor current through LPF [mA] */
    } meas;
	
    struct {
		uint8_t flag;
		uint8_t enable;
		uint16_t maskTimer;			
		uint16_t stallCnt;			/* stall counter */
		uint16_t halfThd;			
		uint16_t threshold;	/* stall threshold [mA] */
		uint16_t obstrCnt;			
    } stall;
	
    struct {
        uint8_t flag;              /* fault flag */
	  uint8_t ocEnable;		
	  uint8_t openEnable;	
	  uint8_t shortEnable;		  
	  uint16_t ocDetectCnt; 
        uint16_t openDetectCnt;  
    } fault;

} motor;
struct {
    tSensorCondition moving;
    uint8_t delay;		
    int8_t filterPeriod;	
    int8_t filterCnt;
    int16_t delta;	
    int16_t thd;			
    int16_t lastDeg;		
} sensor;	

void MotRequestHardStop(void)
{
	motor.requestStop = 1;
}
void MotClearHardStop(void)
{
	motor.requestStop = 0;
}
void MotSetTargetPosition(int16_t targetPos)
{
	int16_t diff;
#if 0
	if (targetPos > (360 * C_GMR_ANGLE_SCALE_FACTOR))
	{
		targetPos -= (360 * C_GMR_ANGLE_SCALE_FACTOR);
	}
#endif	
	motor.pos.target = targetPos;
	if (motor.pos.target >= motor.pos.current)
	{
		diff=motor.pos.target-motor.pos.current;
	}
	else 
	{
		diff=motor.pos.current-motor.pos.target;
	}
	if ((motor.pos.target != motor.pos.lastTarget) || (diff > (int16_t)C_MOT_ON_HYSTERISYS))
	{
		motor.pos.newTarget=1;
		motor.pos.lastTarget = motor.pos.target;
	}
}
void MotSetCurrentPosition(int16_t currentPos)
{
#if 0
	if (currentPos > (360 * C_GMR_ANGLE_SCALE_FACTOR))
	{
		currentPos -= (360 * C_GMR_ANGLE_SCALE_FACTOR);
	}
#endif	
	motor.pos.current = currentPos;
}
int16_t MotGetTargetPosition(void)
{
	return motor.pos.target;
}
int16_t MotGetCurrentPosition(void)
{
	return motor.pos.current;

}
void MotSetParam(int16_t sensorThd,int16_t stallThd)
{
	sensor.thd = sensorThd;
	motor.stall.threshold = stallThd;
}
void MotSetSoftStartAcc(uint16_t acc)
{
motor.softStart.accDuty=acc;
}
void MotSetMaxDuty(uint16_t duty)
{
motor.out.maxDuty=duty;
}
/*
type 0 : all clear
*/
void MotClearStallFlag(uint16_t type)
{
	if (type==0)
	{
		motor.stall.flag=0;
	}
	else if (type==1)
	{
		motor.stall.flag &= (~STALL_MASK_TEMPORARY);
	}
	else
	{
		motor.stall.flag &= (~STALL_MASK_PERMENT);	
	}
}

/*
type 0 : all clear
*/
void MotClearFaultFlag(uint16_t type)
{
	if (type==0)
	{
		motor.fault.flag=0;
		g_e8OverCurrent=0;
		g_e8ShortOcc=0;	
	}
	else if (type==1)
	{
		motor.fault.flag &= (~FAULT_MASK_PHASE_A_OPEN);
	}
	else if (type==2)
	{
		g_e8ShortOcc=0;		
		motor.fault.flag &= (~FAULT_MASK_PHASE_A_SHORT);
	}	
	else if (type==3)
	{
		g_e8OverCurrent=0;		
		motor.fault.flag &= (~FAULT_MASK_OVER_CURRENT);
	}		
	else
	{

	}		
}
tMotState MotGetState(void)
{
	return motor.state;
}
uint8_t MotGetStallState(void)
{
	return motor.stall.flag;
}

uint8_t MotGetFaultState(void)
{
	return motor.fault.flag;
}
uint8_t SensorGetState(void)
{
	return sensor.moving;
}
int16_t SensorGetDelta(void)
{
	return sensor.delta;
}
static uint16_t Mot_dirChange_check(void)
{
	uint16_t flag=0;
#if C_MOT_POLE_POLAR==0
	if (motor.direction==C_DIR_CW)
#else
	if (motor.direction==C_DIR_CCW)
#endif		
	{
		if (motor.pos.Delta < 0) flag=1;
	}
	else
	{
		if (motor.pos.Delta > 0) flag=1;

	}
	return (flag);

}
/**
 * \brief Motor electric diagnostic
 *
 * \return fault flag
 */
static void MotorStallDiag(void)/*100usec */
{
	uint16_t current = get_valve_motCurrent();		

	if (motor.stall.maskTimer < 0xffffu)
	{
		motor.stall.maskTimer += 1;
	}
	if (motor.stall.maskTimer>=1000u)/*100msec*/
	{
		if ((current >= motor.stall.halfThd) && (sensor.delta<sensor.thd) && (sensor.delta>3))		
		{
	
			motor.stall.obstrCnt += 1;
		}
		else
		{
	
			if (motor.stall.obstrCnt > 0) motor.stall.obstrCnt -= 1;
		}
		if (motor.stall.obstrCnt >= 3000u)
		{
if ((motor.stall.enable) && (get_valve_mode() != VALVE_CALIBRATION))
{
			motor.stall.flag |= STALL_MASK_TEMPORARY;
	
u16EventState=MOT_ABSTALL_ERROR;		
u16EventValue=(uint16_t)(current>>3);
u16EventValue|=(uint16_t)((motor.elapsedTime>>8)<<8);
	
}			
		}
		
		if ((current >= motor.stall.threshold) && (sensor.moving==C_STATUS_STOP))	
		{

			motor.stall.stallCnt += 1;
		}
		else
		{

			if (motor.stall.stallCnt > 0) motor.stall.stallCnt -= 1;
		}
		if (motor.stall.stallCnt >= 5000u)
		{
if (motor.stall.enable)	
{
			motor.stall.flag |= STALL_MASK_PERMENT;

u16EventState=MOT_STALL_FAULT;		
u16EventValue=(uint16_t)(current>>3);
u16EventValue|=(uint16_t)((motor.elapsedTime>>8)<<8);				
}			
		}
	}
	else
	{
		motor.stall.stallCnt=0;
		motor.stall.obstrCnt=0;
	}
}

/**
 * \brief Motor electric diagnostic
 *
 * \return fault flag
 */
static void MotorFaultDiag(void)
{
    uint16_t current = get_valve_motCurrent();		

/* open check */
	if (motor.state == MOTION_RUNNING)
	{
		if ((current <= 5) && (sensor.moving==C_STATUS_STOP))	
		{
			motor.fault.openDetectCnt += 1;
		}
		else
		{
			if (motor.fault.openDetectCnt > 0) motor.fault.openDetectCnt -= 1;
		}
		if (motor.fault.openDetectCnt >= 5000u)
		{
if (motor.fault.openEnable)
{
			motor.fault.flag |= FAULT_MASK_PHASE_A_OPEN;

u16EventState=MOT_OPEN_FAULT;		
u16EventValue=(uint16_t)(current>>3);
u16EventValue|=(uint16_t)((motor.elapsedTime>>8)<<8);					
}
		}
	}
	else
	{
		motor.fault.openDetectCnt=0;
	}
/* overcurrent check */	
	if (motor.out.enable)
	{
		if (current >= 1500) 
		{
			motor.fault.ocDetectCnt += 1;
		}
		else
		{
			if (motor.fault.ocDetectCnt > 0) motor.fault.ocDetectCnt -= 1;
		}
		if (motor.fault.ocDetectCnt >= 5000u)
		{
if (motor.fault.ocEnable)
{
			motor.fault.flag |= FAULT_MASK_OVER_CURRENT;

u16EventState=MOT_OC_ERROR;		
u16EventValue=(uint16_t)(current>>3);
u16EventValue|=(uint16_t)((motor.elapsedTime>>8)<<8);				
}
		}
	}
	else
	{
		motor.fault.ocDetectCnt=0;
	}
/* short/oc check */	
	if (g_e8ShortOcc == C_ERR_SHORT_VDS)
	{
		motor.fault.flag |= FAULT_MASK_PHASE_A_SHORT;

u16EventState=MOT_SHORT_FAULT;		
u16EventValue=(uint16_t)(current>>3);
u16EventValue|=(uint16_t)((motor.elapsedTime>>8)<<8);				
	}
	else if (g_e8OverCurrent != 0)
	{
		motor.fault.flag |= FAULT_MASK_OVER_CURRENT;
	}
	else
	{

	}
}

static tMotState motor_state_INIT (void)
{
	tMotState next_state = MOTION_INIT;
	if(motor.initStatus)
	{
		motor.initStatus=0;

	}

	next_state = MOTION_STOPPED;
	return next_state;
}

static tMotState motor_state_STOPPED(void)
{
	tMotState next_state = MOTION_STOPPED;
	uint16_t rState=0;
	if(motor.initStatus)
	{
		motor.initStatus=0;

	}

	motor.out.enable=0;
	motor.out.duty=0;
	motor.pos.posReached = 0;
	if (motor.requestStop != 0)
	{

	}
	else if (motor.pos.newTarget)
	{
		motor.pos.newTarget=0;
#if C_MOT_POLE_POLAR==0		
		if (motor.pos.Delta > 0)
#else
		if (motor.pos.Delta < 0)
#endif
		{
			motor.direction=C_DIR_CW;
		}
		else
		{
			motor.direction=C_DIR_CCW;
		}
		rState=1;
	}
	else
	{

	}

	if (rState != 0)
	{
		pwm_Start(motor.direction,0u);		
		next_state = MOTION_ACC;
	}
	return next_state;
}
/*
C_MOT_STARTDUTY_SET = 10%
motor.softStart.accDuty = 5%
motor.softStart.outThreshold = 90%
ACC duration = (90-10)/5/1ms = 16ms
*/
static tMotState motor_state_ACC (void)/*20250714*/
{
	tMotState next_state = MOTION_ACC;
	uint16_t u16diff;
	if(motor.initStatus)
	{
		motor.initStatus=0;
		motor.out.enable=1;
		motor.out.duty=C_MOT_STARTDUTY_SET;
	}

	if ((motor.requestStop != 0) || (motor.pos.posReached != 0))
	{
		next_state = MOTION_STOPPED;
	}
	else 
	{
		if (motor.pos.Delta >= 0)
		{
			u16diff = motor.pos.Delta;
		}
		else
		{
			u16diff = -motor.pos.Delta;
		}		
		if (motor.softStart.enable)
		{
			if (u16diff <= motor.softStop.inThreshold)
			{
				motor.out.duty += motor.softStart.accDuty;
				if (motor.out.duty >= motor.out.minDuty)
				{
				next_state = MOTION_DEC;
				}
			}	
			else
			{
				motor.out.duty += motor.softStart.accDuty;
				if (motor.out.duty >= motor.softStart.outThreshold)
				{
				next_state = MOTION_RUNNING;
				}
#if DUTY_ADJUST_ENABLE == 1				
				if (motor.out.duty >= motor.out.maxDuty)
				{
				next_state = MOTION_RUNNING;
				}
#endif				
			}
		}
		else
		{

			if (u16diff <= motor.softStop.inThreshold)
			{
				motor.out.duty += motor.softStart.accDuty;
				if (motor.out.duty >= motor.softStart.outThreshold)
				{
				next_state = MOTION_DEC;
				}
			}	
			else
			{
				next_state = MOTION_RUNNING;
			}
		}
	}
	return next_state;
}
static tMotState motor_state_RUNNING (void)
{
	tMotState next_state = MOTION_RUNNING;
	uint16_t u16diff;
	if(motor.initStatus)
	{
		motor.initStatus=0;
		motor.out.enable=1;
	
	}
#if DUTY_ADJUST_ENABLE == 0
	motor.out.duty=C_MOT_MAXDUTY_SET;
#else
	motor.out.duty=motor.out.maxDuty;
#endif	
	if ((motor.requestStop != 0) || (motor.pos.posReached != 0))
	{

		next_state = MOTION_STOPPED;
	}
	else if (Mot_dirChange_check() != 0)
	{
		next_state = MOTION_PAUSE;
	}
#if 0	
	else if (motor.elapsedTime > motor.runTimeOut)
	{
//		next_state = MOTION_FAULT;
		next_state = MOTION_STOPPED;
	}
#endif	
	else
	{
		
		if (motor.softStop.enable)
		{
			if (motor.pos.Delta >= 0)
			{
				u16diff = motor.pos.Delta;
			}
			else
			{
				u16diff = -motor.pos.Delta;
			}			
			if (u16diff <= motor.softStop.inThreshold)
			{
				next_state = MOTION_DEC;
			}
		}
		
	}

	return next_state;
}
static tMotState motor_state_DCC (void)
{
	tMotState next_state = MOTION_DEC;
	uint16_t u16diff=0;
	if(motor.initStatus)
	{
		motor.initStatus=0;
		motor.out.enable=1;
		motor.softStop.completed=0;
	}
	
	if ((motor.requestStop != 0) || (motor.pos.posReached != 0))
	{

		next_state = MOTION_STOPPED;
	}
	else if (Mot_dirChange_check() != 0)
	{
		next_state = MOTION_PAUSE;
		
	}
	else
	{
		if (motor.softStop.completed==0)
		{

#if 0
			motor.out.duty -= motor.softStop.dccDuty;
			if (motor.out.duty < motor.out.minDuty)
			{
				motor.out.duty = motor.out.minDuty;

			}	
#else
			if (motor.pos.Delta >= 0)
			{
				u16diff = motor.pos.Delta;
			}
			else
			{
				u16diff = -motor.pos.Delta;
			}		
			motor.out.duty -= ((u16diff>>1)+motor.softStop.dccDuty);
			if (motor.out.duty < motor.out.minDuty)
			{
				motor.out.duty = motor.out.minDuty;

			}				
#endif
		}
		if (sensor.moving==C_STATUS_STOP)
		{
		
			motor.softStop.completed=1;
			motor.out.duty += ((u16diff>>1)+motor.softStop.dccDuty);
		}
	}

	return next_state;
}
static tMotState motor_state_PAUSE (void)
{
	tMotState next_state = MOTION_PAUSE;

	if(motor.initStatus)
	{
		motor.initStatus=0;
		motor.out.enable=0;

	}
	motor.out.duty = 0;
	if (motor.elapsedTime > 100u)
	{
		next_state = MOTION_STOPPED;

	}

	return next_state;
}

static tMotState motor_state_STALLED (void)
{
	tMotState next_state = MOTION_STALL;

	if(motor.initStatus)
	{
		motor.initStatus=0;
		motor.out.enable=0;

	}
	motor.holdTime=0;
	motor.out.duty=0;
	if(motor.stall.flag == 0u)
	{
		next_state = MOTION_STOPPED;

	}

	return next_state;
}
static tMotState motor_state_FAULT (void)
{
	tMotState next_state = MOTION_FAULT;

	if(motor.initStatus)
	{
		motor.initStatus=0;
		motor.out.enable=0;

	}
	motor.out.duty=0;
	if(motor.fault.flag == 0u)
	{
DIAGNOSTIC_Reset();	
		next_state = MOTION_STOPPED;

	}

	return next_state;
}

void app_mot_init(void)
{

	motor.state=MOTION_STOPPED;
	motor.lastState=MOTION_STOPPED;
	motor.initStatus=1;
	motor.elapsedTime=0;
	motor.direction=C_DIR_NONE;
	motor.lastDirection=C_DIR_NONE;	
	motor.runTimeOut=(5*1000);	
	motor.pos.target=0;
	motor.pos.lastTarget=0;
	motor.pos.current=0;
	motor.pos.newTarget=0;
	motor.pos.posReached=0;
	motor.out.enable=0;
	motor.out.duty=0;
	motor.out.maxDuty=C_MOT_MAXDUTY_SET;
	motor.out.minDuty=C_MOT_MINDUTY_SET;
	motor.softStart.enable=1u;
	motor.softStart.outThreshold=(C_MOT_MAXDUTY_SET * 0.9f);	
#if SOFTSTART_TEST_ENABLE==1
	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.015f);/*50ms=30*/
//	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.008f);/*100ms=16*/
//	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.004f);/*200ms=8*/
//	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.0027f);/*300ms=6*/
//	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.002f);/*400ms=4*/
//	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.0016f);/*500ms=3*/
//	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.0015f);/*533ms*/
#else
	motor.softStart.accDuty=(C_MOT_MAXDUTY_SET * 0.05f);
#endif	
	motor.softStop.enable=1u;
	motor.softStop.completed=0;
	motor.softStop.inThreshold=(4*C_GMR_ANGLE_SCALE_FACTOR);
	motor.softStop.dccDuty=(C_MOT_MAXDUTY_SET * 0.01f);

	motor.stall.flag=0;
	motor.stall.enable=1;	
	motor.stall.maskTimer=0;	
	motor.stall.threshold=800;	/* 1000mA -> 800mA */

	motor.fault.flag=0;
	motor.fault.openEnable=1;
	motor.fault.ocEnable=1;
	motor.fault.ocDetectCnt=0;		
	motor.fault.openDetectCnt=0;

	sensor.delay=0;
	sensor.delta=0;
	sensor.moving=C_STATUS_OFF_;
	sensor.lastDeg=0;
}

/* called by every 1ms */
void app_motor_task(void)
{
	tMotState next_state = motor.state;
	uint16_t voltage = get_valve_voltage();

/*** state machine control ***/
	switch( motor.state )
	{
		case MOTION_INIT:	{next_state = motor_state_INIT(); break; }
		case MOTION_STOPPED:	{next_state = motor_state_STOPPED(); break; }
		case MOTION_ACC:	{next_state = motor_state_ACC(); break; }
		case MOTION_RUNNING:	{next_state = motor_state_RUNNING(); break; }
		case MOTION_DEC:	{next_state = motor_state_DCC(); break; }
		case MOTION_PAUSE:	{next_state = motor_state_PAUSE(); break; }
		case MOTION_STALL:	{next_state = motor_state_STALLED(); break; }
		case MOTION_FAULT:	{next_state = motor_state_FAULT(); break; }
		default: 			
			next_state = MOTION_STOPPED; 
			break; 
	}
	if( next_state != motor.state )
	{
		motor.lastState = motor.state;
		motor.state = next_state;
		motor.initStatus=1u;
		motor.elapsedTime = 0;
	}
	else
	{
		if (motor.elapsedTime < 0xFFFFu) motor.elapsedTime += 1u;
	}
	if (motor.out.enable != 0)
	{
		if (sensor.delay > 0) 
		{
			sensor.delay -= 1;
		}
		if (sensor.delay == 0)
		{
			sensor.filterPeriod += 1;
		}
		
		if (voltage <= 950)
		{
			sensor.thd=10;
			motor.stall.halfThd=550;
			motor.stall.threshold=650;
			motor.out.minDuty=(C_MOT_MAXDUTY_SET * 0.35);
		}			
		else if (voltage <= 1050)
		{
			sensor.thd=11;
			motor.stall.halfThd=600;
			motor.stall.threshold=700;
			motor.out.minDuty=(C_MOT_MAXDUTY_SET * 0.32);
		}
		else if (voltage <= 1150)
		{
			sensor.thd=12;
			motor.stall.halfThd=650;
			motor.stall.threshold=750;
			motor.out.minDuty=(C_MOT_MAXDUTY_SET * 0.29);
		}
		else if (voltage <= 1250)
		{
			sensor.thd=14;
			motor.stall.halfThd=700;
			motor.stall.threshold=800;
			motor.out.minDuty=(C_MOT_MAXDUTY_SET * 0.25);
		}
		else if (voltage <= 1450)
		{
			sensor.thd=16;
			motor.stall.halfThd=800;
			motor.stall.threshold=900;
			motor.out.minDuty=(C_MOT_MAXDUTY_SET * 0.21);
		}		
		else
		{
			sensor.thd=17;
			motor.stall.halfThd=900;
			motor.stall.threshold=1000;
			motor.out.minDuty=(C_MOT_MAXDUTY_SET * 0.18);
		}					
	}
	else
	{
		sensor.delay=50;
		sensor.filterPeriod= 10;
		sensor.filterCnt=0;
		sensor.delta=0;
		sensor.moving=C_STATUS_OFF_;
		sensor.lastDeg = motor.pos.current;
	}
	
	if (sensor.filterPeriod >= 20)/*20msec*/
	{

		sensor.filterPeriod= 0;
		if (motor.pos.current > sensor.lastDeg)
		{
			sensor.delta = motor.pos.current-sensor.lastDeg;
		}
		else
		{
			sensor.delta = sensor.lastDeg-motor.pos.current;
		}
		
		sensor.lastDeg = motor.pos.current;

		if (sensor.delta >= sensor.thd) 
		{
			if (sensor.filterCnt < 3) sensor.filterCnt++;
		}
		else
		{
			if (sensor.filterCnt > -3) sensor.filterCnt--;	
		
		}
		if (sensor.filterCnt >= 2)
		{
		sensor.moving=C_STATUS_RUN;
		}
		else if (sensor.filterCnt <= -2)
		{
	
		sensor.moving=C_STATUS_STOP;
		}
		else {}
	}
}

/* called by every 100us */
void motor_ctrl_handler(void)
{
	uint16_t diff;

	#if LIN_DEBUG_ENABLE
	g_u16DebugData[0] = motor.pos.target;
	g_u16DebugData[1] = motor.pos.current;
	g_u16DebugData[2] = (uint16_t)motor.direction;
	g_u16DebugData[3] = motor.out.duty;
	#endif

	adc_raw_update();
	motor.pos.current = calculate_gmr_angle();

	motor.pos.Delta = (int16_t)(motor.pos.target-motor.pos.current);
	if (motor.pos.Delta >= 0)
	{
		diff = motor.pos.Delta;
	}
	else
	{
		diff = -motor.pos.Delta;
	}
	
	if (diff <= (int16_t)C_MOT_OFF_HYSTERISYS)
	{
		motor.out.enable=0;
		motor.pos.posReached = 1;
		motor.pos.newTarget=0;
	}
	else
	{
	}
    /* motor stall diagnostics */
	if (motor.out.enable)
	{
		MotorStallDiag();	
	}
	else
	{
		motor.stall.maskTimer=0;	
	}
	if (motor.stall.flag != 0u) 
	{

		motor.out.enable=0;
		if (motor.state != MOTION_STALL)
		{
			motor.initStatus=1u;
			motor.elapsedTime = 0;	
			motor.lastState = motor.state;		
			motor.state = MOTION_STALL;
		}
	}	
    /* motor fault diagnostics */
	MotorFaultDiag();
	if (motor.fault.flag != 0)
	{
		motor.holdTime=0;
		motor.out.enable=0;
		if (motor.state != MOTION_FAULT)
		{		
			motor.initStatus=1u;
			motor.elapsedTime = 0;	
			motor.lastState = motor.state;
			motor.state = MOTION_FAULT;
		}
	}

	if (motor.out.enable)
	{
	/* 16384 = 0% */
		pwm_SetDutyCycle(motor.direction,motor.out.duty); 

	}
	else
	{
		if (motor.elapsedTime >= 1000u)
		{
			pwm_Off();
		}
		else
		{
			pwm_Stop();
		}

	}

}

