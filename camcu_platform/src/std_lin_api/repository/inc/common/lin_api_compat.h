/**
 ******************************************************************************
 * @file    lin_api_compat.h
 * @brief   LIN API
 ******************************************************************************
 * @copyright (C) 2016-2018 Melexis N.V. All rights reserved.
 * @internal
 * Melexis N.V. is supplying this software for use with Melexis N.V.
 * processor based microcontrollers only.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * MELEXIS N.V. SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 * @endinternal
 *
 * @details The file purpose is to reduce the impact for the applications
 *          which are based on the previous std LIN API version (< 1.3.0)
 *
 * @ingroup lin_api
 *
 */

#ifndef LIN_API_COMPAT_H
#define LIN_API_COMPAT_H

/*-----------------------------------------------------------------------------
 * Macros
 */
#ifndef LIN_AUTOBAUDRATE_MODE
#define LIN_AUTOBAUDRATE_MODE                   SL_LIN_AUTOBAUDRATE_MODE
#endif

#ifndef HAS_SAVE_CONFIGURATION_FUNCTIONS
#define HAS_SAVE_CONFIGURATION_FUNCTIONS        SL_HAS_SAVE_CONFIGURATION_FUNCTIONS
#endif

#ifndef HAS_SAVE_CONFIGURATION_SERVICE
#define HAS_SAVE_CONFIGURATION_SERVICE          SL_HAS_SAVE_CONFIGURATION_SERVICE
#endif

#ifndef HAS_EVENT_TRIGGERED_FRAMES
#define HAS_EVENT_TRIGGERED_FRAMES              SL_HAS_EVENT_TRIGGERED_FRAMES
#endif

#ifndef HAS_SERIAL_NUMBER_CALLOUT
#define HAS_SERIAL_NUMBER_CALLOUT               SL_HAS_SERIAL_NUMBER_CALLOUT
#endif

#ifndef HAS_CONDITIONAL_CHANGE_NAD_SERVICE
#define HAS_CONDITIONAL_CHANGE_NAD_SERVICE      SL_HAS_CONDITIONAL_CHANGE_NAD_SERVICE
#endif

#ifndef HAS_ASSIGN_NAD_SERVICE
#define HAS_ASSIGN_NAD_SERVICE                  SL_HAS_ASSIGN_NAD_SERVICE
#endif

#ifndef HAS_ASSIGN_FRAME_ID_SERVICE
#define HAS_ASSIGN_FRAME_ID_SERVICE             SL_HAS_ASSIGN_FRAME_ID_SERVICE
#endif

#ifndef HAS_READ_BY_ID_CALLOUT
#define HAS_READ_BY_ID_CALLOUT                  SL_HAS_READ_BY_ID_CALLOUT
#endif

#ifndef HAS_READ_BY_ID_SERVICE
#define HAS_READ_BY_ID_SERVICE                  SL_HAS_READ_BY_ID_SERVICE
#endif

#ifndef LIN_VERSION
#define LIN_VERSION                             SL_LIN_VERSION
#endif

#ifndef LD_N_AS_TIMEOUT
#define LD_N_AS_TIMEOUT                         SL_LD_N_AS_TIMEOUT
#endif

#ifndef LD_N_CR_TIMEOUT
#define LD_N_CR_TIMEOUT                         SL_LD_N_CR_TIMEOUT
#endif

#ifndef ML_NUMBER_OF_DYNAMIC_MESSAGES
#define ML_NUMBER_OF_DYNAMIC_MESSAGES           SL_NUMBER_OF_DYNAMIC_MESSAGES
#endif

#ifndef ML_NODE_CONFIGURATION_INITIALIZER
#define ML_NODE_CONFIGURATION_INITIALIZER       SL_NODE_CONFIGURATION_INITIALIZER
#endif

#endif /* LIN_API_COMPAT_H */
/* EOF */
