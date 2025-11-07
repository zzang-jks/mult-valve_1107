/**
 ******************************************************************************
 * @file    lin_trans_ma.h
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

#ifndef LIN_TRANS_MA_H
#define LIN_TRANS_MA_H

#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif


#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012

/*-----------------------------------------------------------------------------
 * Types definition
 */

/** @name Common definitions */
typedef l_u8 l_m_FrameBuffer_t[8];

/*
 * Transport layer M2S / S2M handlers
 */
/** Request frame source APIs */
typedef union {
    struct {
        l_u8 mrs_Cfg : 1;
        l_u8 mrs_Diag : 1;
    } mapped;

    uint8_t byte;
} ld_MasterReqSource_t;

/** Response frame target APIs */
typedef union {
    struct {
        l_u8 srt_Cfg : 1;
        l_u8 srt_Diag : 1;
    } mapped;

    uint8_t byte;
} ld_SlaveRespTarget_t;

/** Diagnostic request frame handler */
typedef struct {
    l_m_FrameBuffer_t FrameBuffer;    /* TODO: Optimise memory usage */
    ld_MasterReqSource_t Source;
} ld_M2S_TransLayer_t;

/** Diagnostic response frame handler */
typedef struct {
    l_m_FrameBuffer_t FrameBuffer;    /* TODO: Optimise memory usage */
    ld_SlaveRespTarget_t Target;
} ld_S2M_TransLayer_t;


/*-----------------------------------------------------------------------------
 * Function prototypes
 */

/* Transport layer */

#if 0   /* TODO: To be implemented */
/** @name Transport layer */
extern void ld_init_ma (void);
/**@}*/

/** @name Transport layer - Raw API */
extern void ld_put_raw_ma (const l_u8* const data);
extern void ld_get_raw_ma (l_u8* const data);
extern l_u8 ld_raw_tx_status_ma (void);
extern l_u8 ld_raw_rx_status_ma (void);
/**@}*/

/** @name Transport layer - Cooked API */
extern void ld_send_message_ma (l_u16 length, l_u8 NAD, const l_u8* const data);
extern void ld_receive_message_ma (l_u16* const length, l_u8* const NAD, l_u8* const data);
extern l_u8 ld_tx_status_ma (void);
extern l_u8 ld_rx_status_ma (void);
/**@}*/
#endif

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
/** @name Additional helper functions */
extern l_u8 ld_ProcessMasterRequest (l_m_CmdProcEvent_t event);
extern l_u8 ld_ProcessSlaveResponse (l_m_CmdProcEvent_t event);
/**@}*/
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */


/*-----------------------------------------------------------------------------
 * Externals
 */

/* Transport layer TX, RX interfaces */
extern ld_M2S_TransLayer_t M2S_handler;
extern ld_S2M_TransLayer_t S2M_handler;

#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
#endif /* LIN_TRANS_MA_H */

/* EOF */
