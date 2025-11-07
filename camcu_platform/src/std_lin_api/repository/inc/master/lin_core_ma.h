/**
 ******************************************************************************
 * @file    lin_core_ma.h
 * @brief   LIN Master core API
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
 * @ingroup lin_master_api
 *
 */

#ifndef LIN_CORE_MA_H
#define LIN_CORE_MA_H

#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif


/*-----------------------------------------------------------------------------
 * Macros
 */

/* The Master node with version >= 2.0 can support Slave nodes v2.0.
 * The functionality options shall not depend from the Master node version */
#if (MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012) && (MA_EN_ASSIGN_FRAME_ID_COMMAND || \
                                                                 MA_EN_UNASSIGN_FRAME_ID_COMMAND)
#define UNASSIGN_PID_VALUE  0x40u   /**< The PID value used for unassign */
#endif /* (MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012) && (MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND) */


/*-----------------------------------------------------------------------------
 * Master Node Types Definition
 */


/*
 * Frame types
 */

/** Frame types */
typedef enum {
    mft_UnconditionalFrame = 0, /**< Unconditional frame */
    mft_SporadicFrame,          /**< Sporadic frame */
    mft_EventTriggeredFrame     /**< Event triggered frame */
} l_m_FrameType_t;


/*
 * The ::l_ScheduleInstance_t possible FunctionArgs types
 */

/** Frame description type */
typedef struct {
    l_m_FrameType_t FrameType;  /**< Frame type ::l_m_FrameType_t */
    void *Frame;                /**< Pointer to specific frame structure: ::l_m_UnconditionalFrame_t, ::l_m_SporadicFrame_t or ::l_m_EventTriggeredFrame_t */
} l_m_Frame_t;

/** Assign NAD description type */
typedef struct {
    l_u8 InitialNAD;            /**< Slave node Initial NAD */
    l_u8 NewNAD;                /**< Slave node New NAD */
    l_u16 SupplierId;           /**< Slave node Supplier ID */
    l_u16 FunctionId;           /**< Slave node Function ID */
} l_m_AssignNAD_t;

/** Conditional NAD change description type */
typedef struct {
    l_u8 NAD;                   /**< Slave NAD */
    l_u8 Id;                    /**< Identifier */
    l_u8 Byte;                  /**< Byte number (1 is the first byte) */
    l_u8 Mask;                  /**< Bitwise AND byte */
    l_u8 Inv;                   /**< Bitwise XOR byte */
    l_u8 NewNAD;                /**< New NAD if the final result is zero */
} l_m_CondChangeNAD_t;

/** Data dump description type */
typedef struct {
    l_u8 NAD;                   /**< Slave NAD */
    l_u8 Data[5];               /**< Data bytes */
} l_m_DataDump_t;

/** Save configuration description type */
typedef struct {
    l_u8 NAD;                   /**< Slave NAD */
} l_m_SaveConfig_t;

/** Assign PID range description type */
typedef struct {
    l_u8 NAD;                   /**< Slave NAD */
    l_u8 Index;                 /**< Start index */
    l_u8 PIDs[4];               /**< PIDs */
} l_m_AssignFrameIdRange_t;

/** Free format description type */
typedef struct {
    l_u8 Data[8];               /**< Data bytes */
} l_m_FreeFormat_t;

/* The Master node with version >= 2.0 can support Slave nodes v2.0.
 * The functionality options shall not depend from the Master node version */
#if (MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012) && (MA_EN_ASSIGN_FRAME_ID_COMMAND || \
                                                                 MA_EN_UNASSIGN_FRAME_ID_COMMAND)
/** Assign PID description type */
typedef struct {
    l_u8 NAD;                   /**< Slave NAD */
    l_u8 PID;                   /**< The PID value to assign */
    l_u16 SupplierId;           /**< Slave node Supplier ID */
    l_u16 MessageId;            /**< Message ID to assign */
} l_m_AssignFrameId_t;

/** Unassign PID description type */
typedef struct {
    l_u8 NAD;                   /**< Slave NAD */
    l_u16 SupplierId;           /**< Slave node Supplier ID */
    l_u16 MessageId;            /**< Message ID to assign */
} l_m_UnassignFrameId_t;
#endif /* (MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012) && (MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND) */


/*
 * Frame handler function dependent types
 */

/** Frame handler function actions */
typedef enum {
    mfa_UpdateSignals = 0,      /**< Update the Signals from the data inside the Frame (for S2M frames) */
    mfa_FillBuffer,             /**< Prepare the Frame data from the Signals (for M2S frames) */
    mfa_SetFlags,               /**< Set the Flags for Frame & Signals, so it's considered as transmitted(for M2S, S2S frames) or received(for S2M frames) */
    mfa_CheckFlags              /**< Check if some Signal is updated (corresponding Flag is cleared). Is required for Sporadic Frames */
} l_m_FrameAction_t;

/** Frame handler function return values */
typedef enum {
    mfhs_Success = 0,           /**< Success result */
    mfhs_Fail,                  /**< Fail result */
    mfhs_isUpdated              /**< Updated Signal exists */
} l_m_FrameHandlerStatus_t;

/** Pointer to the Frame handler function.
 * The requirements for that function are different and depends on Frame type:
 * For Master API:
 * - M2S frames should provide ::mfa_FillBuffer, ::mfa_SetFlags, ::mfa_CheckFlags (Sporadic frames only ::mft_SporadicFrame) action support;
 * - S2M frames should provide ::mfa_UpdateSignals, ::mfa_SetFlags action support;
 * - S2S frames should provide ::mfa_SetFlags action support.
 * @param[in]   frameAction   requested frame specific action
 * @return status:
 * - 0 - if function execution finished successfully;
 * - 1 - if function execution failed;
 * - 2 - if some Signals were updated (for ::mfa_CheckFlags frameAction only)
 */
typedef l_m_FrameHandlerStatus_t (*l_m_FrameHandler_t)(l_m_FrameAction_t frameAction);


/*
 * Specific frame type description types
 */

/** Unconditional frame */
typedef struct {
    l_u8 PID;                           /**< Protected FID */
    l_u8 DataSize;                      /**< Frame length */
    l_m_FrameHandler_t FrameHandler;    /**< Frame handler function */
    lm_FrameType_t FrameDir;            /**< Frame direction on Master */
    lm_Checksum_t ChecksumType;         /**< Checksum type */
} l_m_UnconditionalFrame_t;

/** Pointer type to ::l_m_UnconditionalFrame_t */
typedef l_m_UnconditionalFrame_t *l_m_pUnconditionalFrame_t;

/* Virtual frames types (no real signal inside); Are used to wrap Unconditional Frames */
/** Sporadic frame
 * @note Sporadic frame has no own flag. It updates relevant Associated frames flags */
typedef struct {
    l_m_pUnconditionalFrame_t *AssociatedFrames;    /**< List of pointers to associated unconditional frames */
    l_u8 FrameCount;                                /**< Length of associated frames list */
} l_m_SporadicFrame_t;

/** Event triggered frame
 * @note Event triggered frame has no own flag. It updates relevant Associated frames flags */
typedef struct {
    l_u8 PID;                                       /**< Protected FID */
    l_u8 FrameCount;                                /**< Length of associated frames list */
    l_Schedule_t *ColResSchdTable;                  /**< Pointer to collision resolving schedule table */
    l_m_pUnconditionalFrame_t *AssociatedFrames;    /**< List of pointers to associated unconditional frames. @note Only frames on which Master is subscribed */
} l_m_EventTriggeredFrame_t;


/*
 *  Shared types
 */

/** Command processing events */
typedef enum {
    /* Frame events */
    cpe_FrameSlotStart,     /**< Frame Slot opened */
    cpe_FrameSlotEnd,       /**< Frame Slot closed */
    cpe_FrameTransmitted,   /**< Frame has been transmitted successfully */
    cpe_FrameReceived,      /**< Frame has been received successfully */
    cpe_RxCollision,        /**< Collision occurs on receive (actual for Event triggered frames) */

    /* Error events */
    cpe_DriverError,        /**< LIN Master driver error (lack of CPU resources or internal Driver error) */
    cpe_FrameError,         /**< Frame processing error */
    cpe_NoResponse          /**< Slave doesn't response */
} l_m_CmdProcEvent_t;

/** Status word helper type */
typedef struct {
    l_bool isFrameConfirmed;      /**< Confirmation of the latest processed frame (def = 0) */
    l_u8 FramesCnt;               /**< Frames processed from the last call (saturated to 2) (def = 0) */
    l_bool isFrameError;          /** Transfer error since the last processed frame (def = 0) */
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
    l_bool isGotoSleep;           /**< Go to sleep request (def = 0) */
    l_bool isGotoSleepConfirmed;  /**< Go to sleep confirmation (def = 0) */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
} l_m_IfcStatusHelper_t;

/** Shared data with LIN Master Driver */
typedef struct {
    lm_Frame_t Frame;   /** Frame structure */
    l_u8 DataBuffer[8]; /** Data buffer */
} l_m_Driver_t;

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
/** Status word type */
typedef union {
    struct {
        l_bool ErrorInResponse    : 1;    /**< Error in response flag */
        l_bool SuccessfulTrans    : 1;    /**< Successful transfer flag */
        l_bool Overrun            : 1;    /**< Few frames were processed since the previous call */
        l_bool GoToSleep          : 1;    /**< Go to Sleep command was transmitted */
#if MA_vLIN_2_1_plus || MA_vISO17987_2016
        l_bool BusActivity        : 1;    /**< Node has detected activity on the bus (isn't used by Master node) */
        l_bool EventTriggered     : 1;    /**< Always 0 (zero) for Slave node */
        l_bool SaveConfig         : 1;    /**< Save configuration request was received (isn't used by Master node) */
        l_u8 unused : 1;
#else /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */
        l_u8 unused : 4;
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */

        l_u8 PID    : 8;    /**< Protected ID */
    } mapped;

    l_u16 word;             /**< Full ifc status word */

} l_m_IfcStatus_t;
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */


/*-----------------------------------------------------------------------------
 *  Function prototypes
 */

/** @name Frame handler helper functions */
extern void l_FillBufferMaster(l_u8 *src, l_u8 size);
/**@}*/


/*-----------------------------------------------------------------------------
 * Externals
 */

extern l_m_Driver_t l_m_Driver;

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
extern l_m_IfcStatusHelper_t l_m_IfcStatusHelper;
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

#endif /* LIN_CORE_MA_H */
/* EOF */
