/**
 * @file
 * @brief configuration for debug interface
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
 * @ingroup application
 */

#ifndef UART_PLOT_CONFIG_H_
#define UART_PLOT_CONFIG_H_

#include <lib_gpio.h>
#include <lib_uart.h>

/** Baudrate (bps) */
#define C_UART_BAUDRATE 2000000UL

/** Pin used for UART-TX */
// #define C_UART_TX_PIN_NR GPIO_IO4
#define C_UART_TX_PIN_NR GPIO_IO6

/** Pin used for UART-RX */
// #define C_UART_RX_PIN_NR GPIO_IO5
#define C_UART_RX_PIN_NR GPIO_IO7

/** Frame length (MLS) */
#define C_UART_FRAME_LENGTH UART_MLS_9Bit

#endif /* UART_PLOT_CONFIG_H_ */

/* EOF */
