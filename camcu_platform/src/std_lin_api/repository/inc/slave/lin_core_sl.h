/**
 ******************************************************************************
 * @file    lin_core_sl.h
 * @brief   LIN Slave core API
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
 * @details The LIN core API handles initialization, processing and a signal based interaction
 *          between the application and the LIN core.
 *
 * @ingroup lin_slave_api
 *
 */

#ifndef LIN_CORE_SL_H
#define LIN_CORE_SL_H

#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif


/*-----------------------------------------------------------------------------
 * Macros
 */

#ifdef MLXCOMP_350_WORKAROUND
#define MLXCOMP_350_AUFRAME_HDLR(frame_handler) {frame_handler}
#else
#define MLXCOMP_350_AUFRAME_HDLR(frame_handler) frame_handler
#endif /* MLXCOMP_350_WORKAROUND */


/*-----------------------------------------------------------------------------
 * Slave Node Types Definition
 */

/*
 * Frame types
 */
/** Frame types */
typedef enum {
    sft_UnconditionalFrame = 0, /**< Unconditional frame */
    sft_EventTriggeredFrame,    /**< Event triggered frame */
    sft_UncondAssociatedFrame   /**< Unconditional frame associated with Event triggered frame */
} l_s_FrameType_t;

/*
 * The instance for Frame list
 */
/** Frame description type */
typedef struct {
    l_s_FrameType_t FrameType;  /**< Frame type ::l_s_FrameType_t */
    void *Frame;                /**< Pointer to specific frame structure: ::l_s_UnconditionalFrame_t, ::l_s_AssociatedUncondFrame_t or ::l_s_EventTriggeredFrame_t */
} l_s_Frame_t;


/*
 * Frame handler function dependent types
 */
/** Frame handler function actions */
typedef enum {
    sfa_UpdateSignals = 0,      /**< Update the Signals from the data inside the Frame */
    sfa_FillBuffer,             /**< Prepare the Frame data from the Signals */
    sfa_FillAsBuffer,           /**< Prepare the Frame data from the Signals for Associated Unconditional frame */
    sfa_SetFlags,               /**< Set the Flags for Frame & Signals, so it's considered as transmitted or received */
    sfa_CheckFlags              /**< Check if some Signal is updated (corresponding Flag is cleared). Is required for Event triggered Frames */
} l_s_FrameAction_t;

/** Frame handler function return values */
typedef enum {
    sfhs_Success = 0,           /**< Success result */
    sfhs_Fail,                  /**< Fail result */
    sfhs_isUpdated              /**< Updated Signal exists */
} l_s_FrameHandlerStatus_t;

/** Pointer to the Frame handler function.
 * The requirements for that function are different and depends on Frame type:
 * For Slave API:
 * - M2S frames should provide ::sfa_UpdateSignals, ::sfa_SetFlags action support;
 * - S2M frames should provide ::sfa_FillBuffer, ::sfa_FillAsBuffer, ::sfa_SetFlags action support;
 * - S2S frames should provide ::sfa_UpdateSignals or ::sfa_FillBuffer, ::sfa_SetFlags action support.
 * @param[in]   frameAction   requested frame specific action
 * @return status:
 * - 0 - if function execution finished successfully;
 * - 1 - if function execution failed;
 * - 2 - if some Signals were updated (for ::sfa_CheckFlags frameAction only)
 */
typedef l_s_FrameHandlerStatus_t (*l_s_FrameHandler_t)(l_s_FrameAction_t frameAction);

/*
 * Frame type specific description types
 */

/** Unconditional frame */
#ifndef MLXCOMP_350_WORKAROUND
typedef struct {
    l_s_FrameHandler_t FrameHandler;  /**< Frame handler function */
} l_s_UnconditionalFrame_t;
#else /* MLXCOMP_350_WORKAROUND */
typedef union {
    l_s_FrameHandler_t FrameHandler;  /**< Frame handler function */
    l_u16 unused;
} l_s_UnconditionalFrame_t;
#endif /* MLXCOMP_350_WORKAROUND */

/** Unconditional frame associated with Event-triggered frame */
#ifndef MLXCOMP_350_WORKAROUND
typedef struct {
    l_u8 PID;                           /**< PID */
    l_s_FrameHandler_t FrameHandler;    /**< Frame handler function */
} l_s_AssociatedUncondFrame_t;
#else /* MLXCOMP_350_WORKAROUND */
typedef struct {
    l_u8 PID;
    union {
        l_s_FrameHandler_t FrameHandler;    /**< Frame handler function */
        l_u16 unused1;
    };
}l_s_AssociatedUncondFrame_t;
#endif /* MLXCOMP_350_WORKAROUND */

/** Event triggered frame */
#ifndef MLXCOMP_350_WORKAROUND
typedef struct {
    l_s_AssociatedUncondFrame_t *AssociatedFrame;   /**< Associated unconditional frame */
} l_s_EventTriggeredFrame_t;
#else /* MLXCOMP_350_WORKAROUND */
typedef union {
    l_s_AssociatedUncondFrame_t *AssociatedFrame;   /**< Associated unconditional frame */
    l_u16 unused;
} l_s_EventTriggeredFrame_t;
#endif /* MLXCOMP_350_WORKAROUND */

/* J2602 (based on LIN2.0) */
#if SL_vSAE_J2602_2012

/** J2602 Error byte type */
typedef union {
    struct {
        l_u8 flagDummy            : 1;  /**< Dummy  */
        l_u8 flagReset            : 1;  /**< The reset has been occurred */
        l_u8 flagReserved_1       : 1;  /**< Reserved */
        l_u8 flagReserved_2       : 1;  /**< Reserved */
        l_u8 flagDataError        : 1;  /**< Data bytes error */
        l_u8 flagChecksumError    : 1;  /**< Checksum error */
        l_u8 flagByteFramingError : 1;  /**< Stop bit error */
        l_u8 flagIDParityError    : 1;  /**< Frame ID parity error */
    } mapped;

    l_u8 J2602ErrorByte;    /**< Whole the error byte */

} l_J2602Error_t;

/** J2602 Status type
 * Contain J2602 Error byte, current reported error, and flag of status existing
 */
typedef struct {
    l_J2602Error_t J2602Error;      /**< Contains all the errors to be transmitted */
    l_u8 J2602CurrentError  : 3;    /**< The error prepared to be reported */
    l_u8 J2602HasStatusFlag : 1;    /**< Defined if the status is transmitted with current frame */
    l_u8 unused             : 4;    /**< Placeholder */
} l_J2602Status_t;
#endif /* SL_vSAE_J2602_2012 */


/*-----------------------------------------------------------------------------
 *  Function prototypes
 */

/** @name Frame handler helper functions */
extern void l_FillBufferSlave(l_u8 *src, l_u8 size);
/**@}*/


/*-----------------------------------------------------------------------------
 *  Externs
 */
extern const l_s_Frame_t frameList[];

#if SL_vSAE_J2602_2012
extern l_J2602Status_t s_J2602Status;
#endif /* SL_vSAE_J2602_2012 */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
extern l_s_IfcStatus_t s_ifcStatus;
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

#endif /* LIN_CORE_SL_H */

/* EOF */

