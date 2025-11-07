/**
 ******************************************************************************
 * @file    lin_trans_ma.c
 * @brief   LIN Master transport layer API
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
 * @details The LIN transport layer is message based. Its intended use is to work as a transport
 *          layer for messages to a diagnostic message parser outside of the LIN device driver.
 *          Two exclusively alternative APIs exist, one raw that allows the application to control
 *          the contents of every frame sent and one cooked that performs the full transport layer function.
 *
 * @ingroup lin_master_api
 *
 */

#include "lin_api.h"

#if LIN_MASTER_API == 1

#include "lin_core_ma.h"
#include "lin_trans_ma.h"
#include "lin_cfg_ma.h"


#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012

/*-----------------------------------------------------------------------------
 * Macros
 */

#define TX_DATA_BUFFER_MAX_LENGTH   8u
#define RX_DATA_BUFFER_MAX_LENGTH   8u


/*-----------------------------------------------------------------------------
 * Types definition
 */


/*
 * Raw API
 */

/** TX RAW API statuses */
typedef enum {
    MA_LD_QUEUE_EMPTY = 0,  /**< The transmit queue is empty */
    MA_LD_QUEUE_AVAILABLE,  /**< The transmit queue contains entries, but is not full */
    MA_LD_QUEUE_FULL,       /**< The transmit queue is full and can not accept further frames */
    MA_LD_TRANSMIT_ERROR    /**< LIN protocol errors occurred during the transfer; initialize and redo the transfer */
} ld_TxRawApiStatus;

/** TX RAW API handler */
typedef struct {
    l_u8 Count;             /**< Current queue length */
    l_u8 MaxCount;          /**< Max queue length */
    ld_TxRawApiStatus Status;
    l_m_FrameBuffer_t FrameBuffer[TX_DATA_BUFFER_MAX_LENGTH];
} ld_TxRawApiHandle_t;

/** RX RAW API statuses */
typedef enum {
    MA_LD_NO_DATA = 0,      /**< The receive queue is empty */
    MA_LD_DATA_AVAILABLE,   /**< The receive queue contains data that can be read */
    MA_LD_RECEIVE_ERROR,    /**< LIN protocol errors occurred during the transfer; initialize and redo the transfer */
} ld_RxRawApiStatus;

/** RX RAW API handler */
typedef struct {
    l_u8 Count;             /**< Current queue length */
    l_u8 MaxCount;          /**< Max queue length */
    ld_RxRawApiStatus Status;
    l_m_FrameBuffer_t FrameBuffer[RX_DATA_BUFFER_MAX_LENGTH];
} ld_RxRawApiHandle_t;


/*
 * Cooked API
 */

/** Cooked API statuses.
 *
 * l_Status meaning for l_TxApiHandle_t:
 * - LD_IN_PROGRESS - The transmission is not yet completed
 * - LD_COMPLETED - The transmission has completed successfully (and you can issue a new ld_send_message call)
 * - LD_FAILED - The transmission ended in an error. The data was only partially sent
 * - LD_N_CR_TIMEOUT - The transmission failed because of a N_As timeout
 *
 * l_Status meaning for l_RxApiHandle_t:
 * - LD_IN_PROGRESS - The reception is not yet completed
 * - LD_COMPLETED - The reception has completed successfully and all information (length, NAD, data) is available
 * - LD_FAILED - The reception ended in an error. The data was only partially received and should not be trusted
 * - LD_N_CR_TIMEOUT - The reception failed because of a N_Cr timeout
 * - LD_WRONG_SN - The reception failed because of an unexpected sequence number
 */
typedef enum {
    MA_LD_IN_PROGRESS = 0,
    MA_LD_COMPLETED,
    MA_LD_FAILED,
    MA_LD_N_CR_TIMEOUT,
    MA_LD_WRONG_SN
} ld_Status;

/** TX Cooked API handler */
typedef struct {
    l_u8 Count;             /**< Data bytes were sent */
    l_u16 Length;           /**< TX data buffer length */
    ld_Status Status;       /**< TX Status */
    l_u8 *DataBytesBuffer;  /**< Pointer to data byte buffer */
} ld_TxApiHandle_t;

/** RX Cooked API handler */
typedef struct {
    l_u8 Count;             /**< Data bytes were sent */
    ld_Status Status;       /**< RX Status */
    l_u16 *Length;          /**< RX data buffer length (max initial and returns actual at the end) */
    l_u8 *NAD;              /**< NAD in the message */
    l_u8 *DataBytesBuffer;  /**< Pointer to data byte buffer */
} ld_RxApiHandle_t;


/*-----------------------------------------------------------------------------
 * Function prototypes
 */

static void ld_ProcessDiagM2S (l_m_CmdProcEvent_t event);
static void ld_ProcessDiagS2M (l_m_CmdProcEvent_t event);


/*-----------------------------------------------------------------------------
 * Variables
 */

/* Transport layer TX, RX interfaces */
ld_M2S_TransLayer_t M2S_handler;
ld_S2M_TransLayer_t S2M_handler;

#if 0   /* TODO: To be implemented */
/* Raw API */
ld_TxRawApiHandle_t txRawApiHandler;
ld_RxRawApiHandle_t rxRawApiHandler;

/* Cooked API */
ld_TxApiHandle_t txApiHandler;
ld_RxApiHandle_t rxApiHandler;
#endif


/*-----------------------------------------------------------------------------
 * Functions
 */

#if 0   /* TODO: To be implemented */
void ld_init (l_ifc_handle iii)
{
    (void)iii;

    /*
     * Raw API
     */

    /* TX handler */
    txRawApiHandler.Count = 0;
    txRawApiHandler.MaxCount = TX_DATA_BUFFER_MAX_LENGTH;
    txRawApiHandler.Status = MA_LD_QUEUE_EMPTY;
    /* RX handler */
    rxRawApiHandler.Count = 0;
    rxRawApiHandler.MaxCount = RX_DATA_BUFFER_MAX_LENGTH;
    rxRawApiHandler.Status = MA_LD_NO_DATA;

    /*
     * Cooked API
     */

    /* TX handler */
    txApiHandler.Status = MA_LD_COMPLETED;
    rxApiHandler.Status = MA_LD_COMPLETED;
}

void ld_put_raw (l_ifc_handle iii, const l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) data;
}

void ld_get_raw (l_ifc_handle iii, l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) data;
}

l_u8 ld_raw_tx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}

l_u8 ld_raw_rx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}

void ld_send_message (l_ifc_handle iii, l_u16 length, l_u8 NAD, const l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) length;
    (void) NAD;
    (void) data;
}

void ld_receive_message (l_ifc_handle iii, l_u16* const length, l_u8* const NAD, l_u8* const data)
{
    /* TODO: To be implemented */
    (void) iii;
    (void) length;
    (void) NAD;
    (void) data;
}

l_u8 ld_tx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}

l_u8 ld_rx_status (l_ifc_handle iii)
{
    /* TODO: To be implemented */
    (void) iii;
    return 0u;
}
#endif

l_u8 ld_ProcessMasterRequest (l_m_CmdProcEvent_t event)
{
    if (M2S_handler.Source.mapped.mrs_Cfg == 1u) { /* Higher priority then Diagnostic frames */
        ld_m_ProcessCfgM2S(event);
    } else if (M2S_handler.Source.mapped.mrs_Diag == 1u) { /* Only when no Config frames requested */
        ld_ProcessDiagM2S(event);
    } else {
        /* Ignore all other cases */
    }
    return 0u;
}

l_u8 ld_ProcessSlaveResponse (l_m_CmdProcEvent_t event)
{
    if (S2M_handler.Target.mapped.srt_Cfg == 1u) {
        ld_m_ProcessCfgS2M(event);
    } else if (S2M_handler.Target.mapped.srt_Diag == 1u) {
        ld_ProcessDiagS2M(event);
    } else {
        /* Ignore all other cases */
    }
    return 0u;
}

static void ld_ProcessDiagM2S (l_m_CmdProcEvent_t event)
{
    /* TODO: To be implemented */
    (void) event;
}

static void ld_ProcessDiagS2M (l_m_CmdProcEvent_t event)
{
    /* TODO: To be implemented */
    (void) event;
}
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

#endif /* LIN_MASTER_API == 1 */
/* EOF */
