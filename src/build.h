/**
 * @file
 * @brief Project configuration
 *
 * @copyright (C) 2020 Melexis N.V. All rights reserved.
 *
 * @section license
 * Melexis N.V. is supplying this code for use with Melexis N.V. processor based microcontrollers only.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.  MELEXIS N.V. SHALL NOT IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Melexis confidential & proprietary
 *
 * @ingroup application
 *
 */

#ifndef BUILD_H_
#define BUILD_H_

#ifdef APP_HAS_DEBUG
#undef CONST
#define CONST
#endif
// Example
// Melexis ldf : 1 x control + 1 x status = 2 frame / slave
#define NR_OF_FRAME_SLAVE 4
// Slave NAD
#define NAD_VALVE0 0x11
#define NAD_VALVE1 0x12
#define NAD_VALVE2 0x13
#define NAD_VALVE3 0x14
#define NAD_VALVE4 0x15 // Add Slave

// #define SUPPORT_MULTI_SLAVE

#define SUPPORT_VALVE0_INDEX_START 0 // 0x01
#define SUPPORT_VALVE1_INDEX_START 1 // SUPPORT_VALVE0_INDEX_START + NR_OF_FRAME_SLAVE
#define SUPPORT_VALVE2_INDEX_START 2 // SUPPORT_VALVE1_INDEX_START + NR_OF_FRAME_SLAVE
#define SUPPORT_VALVE3_INDEX_START 3 // SUPPORT_VALVE2_INDEX_START + NR_OF_FRAME_SLAVE
#define SUPPORT_VALVE4_INDEX_START 4 // SUPPORT_VALVE2_INDEX_START + NR_OF_FRAME_SLAVE	Add Slave

#define uPWM_POL_LOW 0
#define uPWM_POL_HIGH 1
#define PWM_POL_METHOD uPWM_POL_LOW

#endif /* BUILD_H_ */

/* EOF */
