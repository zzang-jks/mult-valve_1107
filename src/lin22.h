/**
 * @file
 * @brief The lin v2.2 module definitions.
 * @internal
 *
 * @copyright (C) 2018-2020 Melexis N.V.
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
 * @ingroup application
 *
 * @details This file contains the definitions of the lin v2.2 module.
 */

#ifndef LIN_22_H_
#define LIN_22_H_

/* ---------------------------
 * Public Defines
 * --------------------------- */

#ifdef APP_HAS_DEBUG
#define DEBUG_DB_B2 1         /* Enabled */
#define DEBUG_DB_B3 1         /* Enabled */
#define DEBUG_DB_B4 1         /* Enabled */
#define DEBUG_DB_B5 1         /* Enabled */
#define DEBUG_DB_B8 1         /* Enabled */
#define DEBUG_BUFFER_SIZE 301 /* The size of the capture buffer */
#endif

extern uint8_t bLinTimeoutActive;
/* ---------------------------------------------
 * Public Function Declarations
 * --------------------------------------------- */

void lin22_Init(void);
void lin22_Stop(void);
void lin22_BackgroundHandler(void);
void lin22_GotoSleep(void);

static INLINE bool lin22_IsLinaaActive(void)
{
    extern bool g_bLinaaActive;
    return g_bLinaaActive;
}

#endif /* LIN_22_H_ */

/* EOF */
