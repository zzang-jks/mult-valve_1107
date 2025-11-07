/*
 * uart.c
 *
 *  Created on: 2025. 10. 21.
 *      Author: Administrator
 */
#include "uart.h"
#include <stdbool.h>
#include <uart_plot.h>

void uartInit(void)
{
	uart_plot_Init();
	uart_plot_Start();
}

void uartTask(void)
{
	uart_plot_SendFrame();
}
