/**
 * @file
 * @brief The uart_plot library definitions.
 * @internal
 *
 * @copyright (C) 2022 Melexis N.V.
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
 */

#ifndef UART_PLOT_H_
    #define UART_PLOT_H_

#include <stdint.h>
#include <stdbool.h>
#include <compiler_abstraction.h>

/* ---------------------------------------------
 * Public Functions
 * --------------------------------------------- */

bool uart_plot_Init(void);
bool uart_plot_Close(void);
bool uart_plot_Start(void);
bool uart_plot_Stop(void);
void uart_plot_SendFrame(void);


/* ---------------------------
 * Public Variables
 * --------------------------- */

/**
 * Check if a transfer is on-going
 * @return  true  if busy
 */
STATIC INLINE bool uart_plot_IsBusy(void);

#ifndef UNITTEST
#include "uart_plot_inline_impl.h"
#endif /* UNITTEST */

#endif  /* UART_PLOT_H_ */

/* EOF */
