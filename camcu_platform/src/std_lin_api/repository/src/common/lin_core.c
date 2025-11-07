/**
 ******************************************************************************
 * @file    lin_core.c
 * @brief   LIN core common API
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
 * @details The LIN core API handles initialization, processing and a signal based interaction
 *          between the application and the LIN core.
 *
 * @ingroup lin_api
 *
 * @defgroup lin_api LIN Common API
 * @{
 * @brief   LIN API
 * @details The LIN API is a network software layer that hides the details of a LIN network
 *          configuration (e.g. how signals are mapped into certain frames) for a user making an application
 *          program for an arbitrary ECU. Instead the user will be provided an API, which is focused on
 *          the signals transported on the LIN network. See lin_api.h for information about the interface.
 * @}
 *
 */

#include "lin_api.h"

#if (LIN_SLAVE_API == 1) || (LIN_MASTER_API == 1)

#include "lin_core.h"


/*-----------------------------------------------------------------------------
 * Variables
 */

const l_ifc_handle dummy;   /**< Dummy interface handler (actual for ROM version Master API realization) */


/*-----------------------------------------------------------------------------
 * Functions
 */

/** Initializes the LIN Module framework.
 * @return success status[l_bool]:
 * @note This is an empty function. The LIN Module, i.e. LIN interface, is initialized and configured in l_ifc_init_sl1
 * - 0 - success;
 * - 1 - fail.
 */
l_bool l_sys_init(void)
{
    return (l_bool)0u;
}


/** Set the flags for region with mask
 * @param[out]  dest    destination pointer
 * @param[in]   mask    used flags mask
 * @param[in]   size    area size
 */
void l_SetFlagsMask(volatile l_u8 *dest, const l_u8 *mask, const l_u8 size)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    for (l_u8 cnt = 0u; cnt < size; cnt++) {
        dest[cnt] |= mask[cnt];
    }
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Check if some flags are updated with mask
 * @param[in]   src     pointer to flags
 * @param[in]   mask    used flags mask
 * @param[in]   size    area size
 * @return bool:
 * - true if some flags are updated (cleared flags)
 * - false otherwise
 */
l_u8 l_IsUpdatedFlagsMask(volatile const l_u8 *src, const l_u8 *mask, const l_u8 size)
{
    l_u8 retVal = 0u;
    l_u8 cnt = 0u;
    do {
        if ((src[cnt] & mask[cnt]) != mask[cnt]) {
            retVal = 1u;
        }
        cnt++;
    } while ((cnt < size) && (retVal != 1u));
    return retVal;
}

#endif /* (LIN_SLAVE_API == 1) || (LIN_MASTER_API == 1) */
/* EOF */
