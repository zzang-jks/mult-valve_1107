/**
 ******************************************************************************
 * @file    lin_trans_sl.h
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

#ifndef LIN_TRANS_SL_H
#define LIN_TRANS_SL_H

#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif


/* Transport layer */

#if 0   /* TODO: To be implemented */
/** @name Transport layer */
extern void ld_init_sl (void);
/**@}*/

/** @name Transport layer - Raw API */
extern void ld_put_raw_sl (const l_u8* const data);
extern void ld_get_raw_sl (l_u8* const data);
extern l_u8 ld_raw_tx_status_sl (void);
extern l_u8 ld_raw_rx_status_sl (void);
/**@}*/

/** @name Transport layer - Cooked API */
extern void ld_send_message_sl (l_u16 length, l_u8 NAD, const l_u8* const data);
extern void ld_receive_message_sl (l_u16* const length, l_u8* const NAD, l_u8* const data);
extern l_u8 ld_tx_status_sl (void);
extern l_u8 ld_rx_status_sl (void);
/**@}*/
#endif

#endif /* LIN_TRANS_SL_H */


/* EOF */
