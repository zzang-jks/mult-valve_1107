/**
 * @file
 * @brief The uart_plot inlines implementation.
 * @internal
 *
 * @copyright (C) 2023 Melexis N.V.
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

#ifndef UART_PLOT_INLINE_IMPL_H_
    #define UART_PLOT_INLINE_IMPL_H_

#include <io_map.h>
#include <io.h>

STATIC INLINE bool uart_plot_IsBusy(void)
{
#ifndef HAS_TWO_UARTS
    return !(IO_GET(PORT_UDMA_STATUS, UDMA_FTR));
#else
    return !(IO_GET(PORT_UDMA0_STATUS, UDMA0_FTR));
#endif
}

#endif  /* UART_PLOT_INLINE_IMPL_H_ */

/* EOF */
