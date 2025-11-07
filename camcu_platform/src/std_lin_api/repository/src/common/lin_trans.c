/**
 ******************************************************************************
 * @file    lin_trans.c
 * @brief   LIN transport layer API
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
 * @ingroup lin_api
 *
 */

#include "lin_api.h"

#if (LIN_SLAVE_API == 1) || (LIN_MASTER_API == 1)

#if LIN_SLAVE_API == 1
#include "lin_trans_sl.h"
#endif /* LIN_SLAVE_API */

#if LIN_MASTER_API == 1
#include "lin_core_ma.h"
#include "lin_trans_ma.h"
#endif /* LIN_MASTER_API */


/*-----------------------------------------------------------------------------
 * Functions
 */

#if 0
void ld_init (l_ifc_handle iii)
{
    switch (iii) {
#if LIN_SLAVE_API == 1
        case ifcSlave:
            ld_init_sl();
            break;
#endif /* LIN_SLAVE_API */
#if LIN_MASTER_API == 1
        case ifcMaster:
            ld_init_ma();
            break;
#endif /* LIN_MASTER_API */
        default:
            break;
    }
}

void ld_put_raw (l_ifc_handle iii, const l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) data;
}

void ld_get_raw (l_ifc_handle iii, l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) data;
}

l_u8 ld_raw_tx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}

l_u8 ld_raw_rx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}

void ld_send_message (l_ifc_handle iii, l_u16 length, l_u8 NAD, const l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) length;
    (void) NAD;
    (void) data;
}

void ld_receive_message (l_ifc_handle iii, l_u16* const length, l_u8* const NAD, l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) length;
    (void) NAD;
    (void) data;
}

l_u8 ld_tx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}

l_u8 ld_rx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}
#endif

#endif /* (LIN_SLAVE_API == 1) || (LIN_MASTER_API == 1) */
/* EOF */
