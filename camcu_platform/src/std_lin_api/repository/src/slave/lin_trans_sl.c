/**
 ******************************************************************************
 * @file    lin_trans_sl.c
 * @brief   LIN Slave transport layer API
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
 * @details The LIN transport layer is message based. Its intended use is to work as a transport
 *          layer for messages to a diagnostic message parser outside of the LIN device driver.
 *          Two exclusively alternative APIs exist, one raw that allows the application to control
 *          the contents of every frame sent and one cooked that performs the full transport layer function.
 *
 * @ingroup lin_slave_api
 *
 */

#include "lin_api.h"

#if LIN_SLAVE_API == 1

#include "lin_trans_sl.h"


/*-----------------------------------------------------------------------------
 * Types definition
 */


/*-----------------------------------------------------------------------------
 * Function prototypes
 */


/*-----------------------------------------------------------------------------
 * Variables
 */


/*-----------------------------------------------------------------------------
 * Functions
 */

#endif /* LIN_SLAVE_API == 1 */
/* EOF */

