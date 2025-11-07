/**
 * @file
 * @brief The uart_plot library.
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

#include <atomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys_tools.h>
#include <io.h>
#include <itc_helper.h>
#include <lib_gpio.h>
#include <lib_uart.h>
#include "fw_ints_prio.h"
#include "uart_plot_config.h"
#include "uart_plot.h"

/* ---------------------------------------------
 * Definitions
 * --------------------------------------------- */

/*
 * https://jira.melexis.com/jira/browse/MLX81332-263
 */
#if defined MLX81332 || defined MLX81334
#ifndef GPIO_UART_OUT
#define GPIO_UART_OUT 15
#endif
#endif

/** 2 bytes header + 1 byte size + 8 bytes data + 1 byte checksum */
#define C_UART_BUF_SZ 10u
#define C_UART_PLOT_DATA_POS (1u)
#define C_UART_PLOT_DATA_LENGTH (8u)
#define C_UART_PLOT_CRC_POS (C_UART_PLOT_DATA_POS + C_UART_PLOT_DATA_LENGTH)

/** Default Frame Start Pattern */
#define C_SERIAL_PLOT_HEADER_BYTE0 0xAA


/* ---------------------------------------------
 * Special Types
 * --------------------------------------------- */

typedef enum UartTx_e
{
    e_uart_tx_low_bit_low_byte_first = 0, /**< first low bit and first low byte. Default. The only valid setting*/
    e_uart_tx_low_bit_high_byte_first,    /**< first low bit and first high byte*/
    e_uart_tx_high_bit_low_byte_first,    /**< first high bit and first low byte*/
    e_uart_tx_high_bit_high_byte_first,   /**< first high bit and first high byte*/
    e_uart_tx_NumberOf                    /**< number of available settings for this enumeration*/
} UartTx_t;

typedef struct UartConfig_s
{
    uint32_t baudrate;  /**< in bits per second */
    UartTx_t bit_byte_order;  /**< bit and byte order used in the uart protocol */
    GpioIo_t tx_pin;  /**< GPIO pin used for Tx */
    GpioIo_t rx_pin;  /**< GPIO pin used for Rx */
} UartConfig_t;

typedef union
{
    uint8_t buf[C_UART_BUF_SZ] __attribute__((aligned(2)));
    struct __attribute__((packed))
    {
        uint8_t header;
        uint16_t data[4];
        uint8_t cs;
    } s;
} uart_plot_txMsg_t;

ASSERT(sizeof(uart_plot_txMsg_t) == C_UART_BUF_SZ);


/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */
#pragma space dp
uint16_t g_u16TxAddress[4] = { 0 };  /**< address of 1..4 words to transmit */
#pragma space none

static uart_plot_txMsg_t l_uart_plot_txMsg;   /**< UART Transmit frame */
static uint8_t l_au8UartRdBufA[C_UART_BUF_SZ] __attribute__((aligned(2)));  /**< UART DMA Receive buffer */
static uint8_t l_au8UartRdBufB[C_UART_BUF_SZ] __attribute__((aligned(2)));  /**< UART DMA Receive buffer */
static bool l_bCommandReady = false;  /**< A new command has been received */
static uint16_t l_u16Payload[4];  /**< 1..4 words received */
static volatile uint16_t l_uart_plot_dummy = 0x55;


/* ---------------------------------------------
 * Local Functions Declaration
 * --------------------------------------------- */

STATIC bool uart_Init(const UartConfig_t * const config);
STATIC void uart_Close(void);
STATIC void uart_Start(void);
STATIC void uart_Stop(void);


/* ---------------------------------------------
 * Public Functions
 * --------------------------------------------- */

/**
 * Initialize plot / uart HW resources
 * @retval  true  in case of success
 */
bool uart_plot_Init(void)
{
    bool bRetVal;

    l_uart_plot_txMsg.s.header = C_SERIAL_PLOT_HEADER_BYTE0;  /* header */
    g_u16TxAddress[0] = (uint16_t)&l_uart_plot_dummy;
    g_u16TxAddress[1] = (uint16_t)&l_uart_plot_dummy;
    g_u16TxAddress[2] = (uint16_t)&l_uart_plot_dummy;
    g_u16TxAddress[3] = (uint16_t)&l_uart_plot_dummy;

    /* configure UART */
    static const UartConfig_t uart_tx_config = {
        .baudrate = C_UART_BAUDRATE,  /* bits per second */
        .bit_byte_order = e_uart_tx_low_bit_high_byte_first,  /* only this setting is supported */
        .tx_pin = C_UART_TX_PIN_NR,
        .rx_pin = C_UART_RX_PIN_NR
    };
    bRetVal = uart_Init(&uart_tx_config);

    return bRetVal;
}

/**
 * Disable UART
 * @retval  true  in case of success
 */
bool uart_plot_Close(void)
{
    bool bRetVal = true;

    uart_Close();

    return bRetVal;
}

/**
 * Start plot processing
 * @retval  true  in case of success
 */
bool uart_plot_Start(void)
{
    bool bRetVal = true;

    uart_Start();

    return bRetVal;
}

/**
 * Stop plot processing
 */
bool uart_plot_Stop(void)
{
    bool bRetVal = true;

    uart_Stop();

    return bRetVal;
}

/**
 * Transmit a frame of format
 * byte0  byte1..byte8 byte9
 * header data         checksum
 */
void uart_plot_SendFrame(void)
{
    uint16_t tmp;

    /* Checksum byte is sent at the end of each frame.
     * It should be the least significant byte of the byte sum of all data bytes.
     */
    l_uart_plot_txMsg.s.cs = 0u;
    tmp = *(uint16_t*)(g_u16TxAddress[0]);
    l_uart_plot_txMsg.s.data[0] = tmp;
    l_uart_plot_txMsg.s.cs += tmp >> 8;
    l_uart_plot_txMsg.s.cs += tmp;
    tmp = *(uint16_t*)(g_u16TxAddress[1]);
    l_uart_plot_txMsg.s.data[1] = tmp;
    l_uart_plot_txMsg.s.cs += tmp >> 8;
    l_uart_plot_txMsg.s.cs += tmp;
    tmp = *(uint16_t*)(g_u16TxAddress[2]);
    l_uart_plot_txMsg.s.data[2] = tmp;
    l_uart_plot_txMsg.s.cs += tmp >> 8;
    l_uart_plot_txMsg.s.cs += tmp;
    tmp = *(uint16_t*)(g_u16TxAddress[3]);
    l_uart_plot_txMsg.s.data[3] = tmp;
    l_uart_plot_txMsg.s.cs += tmp >> 8;
    l_uart_plot_txMsg.s.cs += tmp;

#ifndef HAS_TWO_UARTS
    uint8_t udma_ctrl = IO_BYTE_HOST(PORT_UDMA_CTRL, UDMA_TXSTART);
    udma_ctrl &= ~(1u << IO_OFFSET(PORT_UDMA_CTRL, UDMA_TXSTART));
    IO_BYTE_HOST(PORT_UDMA_CTRL, UDMA_TXSTART) = udma_ctrl;
    udma_ctrl |= (1u << IO_OFFSET(PORT_UDMA_CTRL, UDMA_TXSTART));
    IO_BYTE_HOST(PORT_UDMA_CTRL, UDMA_TXSTART) = udma_ctrl;
#else
    uint8_t udma_ctrl = IO_BYTE_HOST(PORT_UDMA0_CTRL, UDMA0_TXSTART);
    udma_ctrl &= ~(1u << IO_OFFSET(PORT_UDMA0_CTRL, UDMA0_TXSTART));
    IO_BYTE_HOST(PORT_UDMA0_CTRL, UDMA0_TXSTART) = udma_ctrl;
    udma_ctrl |= (1u << IO_OFFSET(PORT_UDMA0_CTRL, UDMA0_TXSTART));
    IO_BYTE_HOST(PORT_UDMA0_CTRL, UDMA0_TXSTART) = udma_ctrl;

#endif
}


/* ---------------------------------------------
 * Interrupts
 * --------------------------------------------- */

/**
 * UART ISR Frame Received
 */
#ifndef HAS_TWO_UARTS
INTERRUPT void _UDFR_INT(void)
#else
INTERRUPT void _UDFR0_INT(void)
#endif
{
    uint8_t * pu8UartRxBuf;

#ifndef HAS_TWO_UARTS
    if (IO_GET(PORT_UDMA_STATUS, UDMA_RD_BUFFER_VALID) == 0u)
#else
    if (IO_GET(PORT_UDMA0_STATUS, UDMA0_RD_BUFFER_VALID) == 0u)
#endif
    {
        pu8UartRxBuf = (uint8_t *)&l_au8UartRdBufB;
    }
    else
    {
        pu8UartRxBuf = (uint8_t *)&l_au8UartRdBufA;
    }

    if ((pu8UartRxBuf[0] == C_SERIAL_PLOT_HEADER_BYTE0))
    {
        uint8_t checksum = 0u;
        for (uint8_t index = 0u; index < C_UART_PLOT_DATA_LENGTH; index++)
        {
            checksum += pu8UartRxBuf[1u + index];
        }

        if (checksum == pu8UartRxBuf[C_UART_PLOT_CRC_POS])
        {
            for (uint8_t index = 0u; index < C_UART_PLOT_DATA_LENGTH / 2; index++)
            {
                uint16_t addr = (pu8UartRxBuf[1u + (index * 2u) + 1u] << 8) | pu8UartRxBuf[1u + (index * 2u)];
                if (addr != 0)
                {
                    l_u16Payload[index] = addr;
                }
                else
                {
                    l_u16Payload[index] = (uint16_t)&l_uart_plot_dummy;
                }
            }

            l_bCommandReady = true;
            if (l_bCommandReady)
            {
                memcpy(g_u16TxAddress, l_u16Payload, C_UART_PLOT_DATA_LENGTH);
                l_bCommandReady = false;
            }
        }
    }
    else
    {
        /* we lost synchronization, repair */
        uart_Stop();
        uart_Start();
    }
}

/* ---------------------------------------------
 * Local Functions
 * --------------------------------------------- */

/**
 * Initialize UART
 * @param  config UART configuration struct
 * @return succes = true
 */
STATIC bool uart_Init(const UartConfig_t * const config)
{
    const UartTx_t bit_byte_order = config->bit_byte_order;
    bool rc = true;

    /* select the pin to be used as UART Tx port */
    rc = gpio_configureOutput(config->tx_pin, GPIO_UART_OUT);

#ifndef HAS_TWO_UARTS
    IO_SET(PORT_COMM_CFG1, UART_RX_SEL, config->rx_pin); /* Select rx_pin as UART0 input */
#else
    IO_SET(PORT_COMM_CFG1, UART0_RX_SEL, config->rx_pin); /* Select rx_pin as UART0 input */
#endif

#ifdef HAS_TWO_UARTS
    IO_SET(PORT_IO_UART_SEL, IO_UART_SEL, 0x0000);  /* Select UART0 for all IO pins */
#endif

    if (rc) /* output pin config succeeded */
    {
#ifndef HAS_TWO_UARTS
        UART_FrameLength(C_UART_FRAME_LENGTH);
        UART_TxFormat(bit_byte_order);
#else
        UART0_FrameLength(C_UART_FRAME_LENGTH);
        UART0_TxFormat(bit_byte_order);
#endif

        /* setup UART baudrate */
        const uint16_t baudrate_divider = UART_GetRate(config->baudrate);
#ifndef HAS_TWO_UARTS
        UART_SetBaudrate(baudrate_divider - 1);  /* IMPORTANT returned baudrate must be decremented by 1 */
#else
        UART0_SetBaudrate(baudrate_divider - 1);  /* IMPORTANT returned baudrate must be decremented by 1 */
#endif

#ifndef HAS_TWO_UARTS
        /* Configure UART DMA ports */
        IO_SET(PORT_UDMA_RDA, UDMA_RDA, (uint16_t)((void *)&l_au8UartRdBufA[0]));
        IO_SET(PORT_UDMA_RDB, UDMA_RDB, (uint16_t)((void *)&l_au8UartRdBufB[0]));
        IO_SET(PORT_UDMA_TX, UDMA_TX, (uint16_t)((void *)l_uart_plot_txMsg.buf));

        /* Length transmit buffer in Bytes (minus 1) */
        IO_SET(PORT_UDMA_SIZE, UDMA_SIZTX, (C_UART_BUF_SZ - 1u), UDMA_SIZRX, (C_UART_BUF_SZ - 1u));

        /* Configure UART DMA mode */
        IO_SET(PORT_UDMA_CTRL, UDMA_LSB_FIRST, 1, UDMA_LOWBYTE_FIRST, 1);
#else
        /* Configure UART DMA ports */
        IO_HOST(PORT_UDMA0_RDA, UDMA0_RDA) = (uint16_t)((void *)&l_au8UartRdBufA[0]);
        IO_HOST(PORT_UDMA0_RDB, UDMA0_RDB) = (uint16_t)((void *)&l_au8UartRdBufB[0]);
        IO_HOST(PORT_UDMA0_TX, UDMA0_TX) = (uint16_t)((void *)l_uart_plot_txMsg.buf);

        /* Length transmit buffer in Bytes (minus 1) */
        IO_SET(PORT_UDMA0_SIZE, UDMA0_SIZTX, (C_UART_BUF_SZ - 1u), UDMA0_SIZRX, (C_UART_BUF_SZ - 1u));

        IO_SET(PORT_UDMA0_CTRL, UDMA0_LSB_FIRST, 1, UDMA0_LOWBYTE_FIRST, 1);
#endif

        #ifndef UNITTEST
        ENTER_SECTION(SYSTEM_MODE);
#ifndef HAS_TWO_UARTS
        Itc_Clear(UDFR);
        Itc_Enable(UDFR);
        Itc_SetPrio(UDFR, PRIO_UDFR_INT);
#else
        Itc_Clear(UDFR0);
        Itc_Enable(UDFR0);
        Itc_SetPrio(UDFR0, PRIO_UDFR0_INT);
#endif
        EXIT_SECTION();
        #endif
    }

    return rc;
}

/**
 * start UART DMA operation
 */
STATIC void uart_Start(void)
{
#ifndef HAS_TWO_UARTS
    UART_RxEnable();
    UART_TxEnable();
    IO_SET(PORT_UDMA_CTRL, UDMA_TXSTART, 1, UDMA_EN, 1);  /* Enable & Start UART DMA */
#else
    UART0_RxEnable();
    UART0_TxEnable();
    IO_SET(PORT_UDMA0_CTRL, UDMA0_TXSTART, 1, UDMA0_EN, 1);  /* Enable & Start UART DMA */
#endif
}

/**
 * reset UART and stop DMA
 * bring the UART into the reset state (as specified in the DS)
 */
STATIC void uart_Close(void)
{
    /* reset UART itself */
#ifndef HAS_TWO_UARTS
    IO_SET(PORT_UDMA_CTRL, UDMA_EN, 0);  /* Disable UART DMA */
    UART_ResetRegister();
    UART_TxDisable();
    UART_RxDisable();
#else
    IO_SET(PORT_UDMA0_CTRL, UDMA0_EN, 0);  /* Disable UART DMA */
    UART0_ResetRegister();
    UART0_TxDisable();
    UART0_RxDisable();
#endif

    /* Disable interrupts */
    #ifndef UNITTEST
    ENTER_SECTION(SYSTEM_MODE);
#ifndef HAS_TWO_UARTS
    Itc_Disable(UDFR);
    Itc_Clear(UDFR);
#else
    Itc_Disable(UDFR0);
    Itc_Clear(UDFR0);
#endif
    EXIT_SECTION();
    #endif
}

/**
 * Stop the UART
 */
STATIC void uart_Stop(void)
{
#ifndef HAS_TWO_UARTS
    UART_RxDisable();
    UART_TxDisable();
#else
    UART0_RxDisable();
    UART0_TxDisable();
#endif
}

#ifdef UNITTEST
#include "uart_plot_inline_impl.h"
#endif /* UNITTEST */

/* EOF */
