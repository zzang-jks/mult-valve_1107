/**
 * @file
 * @brief The software timer library definitions.
 * @internal
 *
 * @copyright (C) 2019-2021 Melexis N.V.
 *
 * Melexis N.V. is supplying this code for use with Melexis N.V. processor based microcontrollers only.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.  MELEXIS N.V. SHALL NOT IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * @endinternal
 *
 * @ingroup libraries
 *
 * @brief
 * @details
 * @{
 */

#ifndef SWTIMER_INLINE_IMPL_H
    #define SWTIMER_INLINE_IMPL_H

STATIC INLINE uint16_t swtimer_getCurrent(swtimer_id_t a_e8Id)
{
    extern swtimer_config_t g_pTimer[];
    return (g_pTimer[a_e8Id].u15Current);
}

STATIC INLINE uint16_t swtimer_getPeriod(swtimer_id_t a_e8Id)
{
    extern swtimer_config_t g_pTimer[];
    return (g_pTimer[a_e8Id].u15Load);
}

#endif /* SWTIMER_INLINE_IMPL_H */
/*/ @} */
