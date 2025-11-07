/*
 * AppLin.c
 *
 *  Created on: 2025. 5. 29.
 *      Author: mctp
 */
#include <stdbool.h>
#include <lin_api.h>
#include "fw_mls_api.h"
#include "lin22.h"
#include "defines.h"
#include "AppLin.h"
#include "AppValve.h"

uint8_t Fwv_lin_sleep_enable = 0;
uint8_t Fwv_lin_frame_Error = 0;
uint8_t Fwv_Request_Event = 0;
uint8_t Fwv_Response_Event = 0;

void AppLinInit(void)
{
	Fwv_lin_sleep_enable = 0;
	Fwv_lin_frame_Error = 0;
}

void AppLinTask(void)
{

	if (g_u8LinErrorCnt > (uint8_t)3u)
	{
		Fwv_lin_frame_Error = 1;
	}

	/* VPC_Fwv_Master Frame handling */
	if (l_flg_tst_f_VPC_Fwv_Ctrl() != 0u)
	{
		l_flg_clr_f_VPC_Fwv_Ctrl();
		ValveLinGetCommand();
		Fwv_Request_Event = 1;
		g_u8LinErrorCnt = 0;
		Fwv_lin_frame_Error = 0;
	}

	/* VPC_Fwv_Slave Frame handling */
	if (l_flg_tst_f_VPC_Fwv_Resp() != 0u)
	{
		l_flg_clr_f_VPC_Fwv_Resp();
		ValveLinUpdateSignals();
		Fwv_Response_Event = 1;
		g_u8LinErrorCnt = 0;
		Fwv_lin_frame_Error = 0;
	}
#if LIN_DEBUG_ENABLE
	/* DEBUG1_FRAME handling */
	if (l_flg_tst_f_DEBUG1_FRAME() != 0u)
	{
		l_flg_clr_f_DEBUG1_FRAME();

		l_u8_wr_DEBUG1_U8_1((uint8_t)(g_u16DebugData[0]));
		l_u8_wr_DEBUG1_U8_2((uint8_t)(g_u16DebugData[0] >> 8));
		l_u8_wr_DEBUG1_U8_3((uint8_t)(g_u16DebugData[1]));
		l_u8_wr_DEBUG1_U8_4((uint8_t)(g_u16DebugData[1] >> 8));
		l_u8_wr_DEBUG1_U8_5((uint8_t)(g_u16DebugData[2]));
		l_u8_wr_DEBUG1_U8_6((uint8_t)(g_u16DebugData[2] >> 8));
		l_u8_wr_DEBUG1_U8_7((uint8_t)(g_u16DebugData[3]));
		l_u8_wr_DEBUG1_U8_8((uint8_t)(g_u16DebugData[3] >> 8));
	}
#endif
	/* do lin period (non time critical) stuff */
	lin22_BackgroundHandler();
}

void AppLinSleepEnter(void)
{
	Fwv_lin_sleep_enable = 1;
	lin22_GotoSleep(); /* switch to sleep mode */
}
uint8_t LinGetCommState(void)
{
	return Fwv_lin_frame_Error;
}
