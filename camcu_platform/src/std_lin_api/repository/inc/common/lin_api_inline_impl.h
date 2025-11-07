/**
 ******************************************************************************
 * @file    lin_api_inline_impl.h
 * @brief   LIN API
 ******************************************************************************
 * @copyright (C) 2018 Melexis N.V. All rights reserved.
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
 * @details Inline functions for lin_api.h file
 *
 * @ingroup lin_api
 *
 */

STATIC INLINE void l_sys_irq_restore_pointer (const l_irqmask *p)
{
    l_sys_irq_restore(*p);
}
