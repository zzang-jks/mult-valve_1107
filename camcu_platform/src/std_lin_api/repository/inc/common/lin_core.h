/**
 ******************************************************************************
 * @file    lin_core.h
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
 */

#ifndef LIN_CORE_H
#define LIN_CORE_H


/*-----------------------------------------------------------------------------
 *  Function prototypes
 */

/** @name Frame handler helper functions */
extern void l_SetFlagsMask(volatile l_u8 *dest, const l_u8 *mask, const l_u8 size);
extern l_u8 l_IsUpdatedFlagsMask(volatile const l_u8 *src, const l_u8 *mask, const l_u8 size);
/**@}*/


#endif /* LIN_CORE_H */
/* EOF */
