/**
 ******************************************************************************
 * @file    lin_api_sl.h
 * @brief   LIN Slave API
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
 * @details The LIN API is a network software layer that hides the details of a LIN network
 *          configuration (e.g. how signals are mapped into certain frames) for a user making an application
 *          program for an arbitrary ECU. Instead the user will be provided an API, which is focused on
 *          the signals transported on the LIN network.
 *
 *          The API is split in three sections:
 *          -# LIN core API (lin_core_sl.h)
 *          -# LIN node configuration and identification API (lin_cfg_sl.h)
 *          -# LIN transport layer API (optional) (lin_trans_sl.h)
 *
 * @ingroup lin_slave_api
 *
 */

#ifndef LIN_API_SL_H
#define LIN_API_SL_H

#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif


/*
 * Macros
 */

/* The SL_HAS_SAVE_CONFIGURATION_FUNCTIONS is disabled by default */
#ifndef SL_HAS_SAVE_CONFIGURATION_FUNCTIONS
#define SL_HAS_SAVE_CONFIGURATION_FUNCTIONS 0
#endif /* SL_HAS_SAVE_CONFIGURATION_FUNCTIONS */

#ifndef SL_HAS_OTHER_NAD_DIAG_CALLOUT
#define SL_HAS_OTHER_NAD_DIAG_CALLOUT 0
#endif /* SL_HAS_OTHER_NAD_DIAG_CALLOUT */

/* Helper macro for configurations */
#define SL_EN_SAVE_CONFIG_FUNC            IS_ENABLED_CONF(SL_HAS_SAVE_CONFIGURATION_FUNCTIONS)
#define SL_EN_EVENT_TRIGGERED_FRAMES      IS_ENABLED_CONF(SL_HAS_EVENT_TRIGGERED_FRAMES)
#define SL_EN_SAVE_CONFIG_SERVICE         IS_ENABLED_CONF(SL_HAS_SAVE_CONFIGURATION_SERVICE)
#define SL_EN_SERIAL_NUMBER_CALLOUT       IS_ENABLED_CONF(SL_HAS_SERIAL_NUMBER_CALLOUT)
#define SL_EN_COND_CHANGE_NAD_SERVICE     IS_ENABLED_CONF(SL_HAS_CONDITIONAL_CHANGE_NAD_SERVICE)
#define SL_EN_ASSIGN_NAD_SERVICE          IS_ENABLED_CONF(SL_HAS_ASSIGN_NAD_SERVICE)
#define SL_EN_ASSIGN_FRAME_ID_SERVICE     IS_ENABLED_CONF(SL_HAS_ASSIGN_FRAME_ID_SERVICE)
#define SL_EN_READ_BY_ID_CALLOUT          IS_ENABLED_CONF(SL_HAS_READ_BY_ID_CALLOUT)
#define SL_EN_READ_BY_ID_SERVICE          IS_ENABLED_CONF(SL_HAS_READ_BY_ID_SERVICE)
#define SL_EN_OTHER_NAD_DIAG_CALLOUT      IS_ENABLED_CONF(SL_HAS_OTHER_NAD_DIAG_CALLOUT)
#define SL_EN_READ_BY_ID_BIT_TIMING_TEST  IS_ENABLED_CONF(SL_HAS_READ_BY_ID_BIT_TIMING_TEST)
#define SL_EN_NCF_LDF_VERSION             IS_ENABLED_CONF(SL_HAS_NCF_LDF_VERSION)

#define SL_DIS_SAVE_CONF_FUNC             IS_DISABLED_CONF(SL_HAS_SAVE_CONFIGURATION_FUNCTIONS)
#define SL_DIS_EVENT_TRIG_FRAMES          IS_DISABLED_CONF(SL_HAS_EVENT_TRIGGERED_FRAMES)
#define SL_DIS_SAVE_CONF_SERVICE          IS_DISABLED_CONF(SL_HAS_SAVE_CONFIGURATION_SERVICE)
#define SL_DIS_SERIAL_NUMBER_CALLOUT      IS_DISABLED_CONF(SL_HAS_SERIAL_NUMBER_CALLOUT)
#define SL_DIS_COND_CHANGE_NAD_SERVICE    IS_DISABLED_CONF(SL_HAS_CONDITIONAL_CHANGE_NAD_SERVICE)
#define SL_DIS_ASSIGN_NAD_SERVICE         IS_DISABLED_CONF(SL_HAS_ASSIGN_NAD_SERVICE)
#define SL_DIS_ASSIGN_FRAME_ID_SERVICE    IS_DISABLED_CONF(SL_HAS_ASSIGN_FRAME_ID_SERVICE)
#define SL_DIS_READ_BY_ID_CALLOUT         IS_DISABLED_CONF(SL_HAS_READ_BY_ID_CALLOUT)
#define SL_DIS_READ_BY_ID_SERVICE         IS_DISABLED_CONF(SL_HAS_READ_BY_ID_SERVICE)
#define SL_DIS_READ_BY_ID_BIT_TIMING_TEST IS_DISABLED_CONF(SL_HAS_READ_BY_ID_BIT_TIMING_TEST)
#define SL_DIS_NCF_LDF_VERSION            IS_DISABLED_CONF(SL_HAS_NCF_LDF_VERSION)

/* The diag. communication isn't supported by LIN13 */
#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012

#if SL_EN_READ_BY_ID_CALLOUT
#define DF_DATA_IN_SINGLEFRAME  5u
#define DF_DATA_IN_FIRSTFRAME   4u
#define DF_DATA_IN_CONFRAME     6u
#define DF_NON_FIXED_DATA       6u

#define LD_NEGATIVE_RESPONSE ((l_u8)respNegativeResponse)
#define LD_POSITIVE_RESPONSE ((l_u8)respUserDefIdCallout)
#define LD_NO_RESPONSE       ((l_u8)respNoResponse)
#endif /* SL_EN_READ_BY_ID_CALLOUT */


/*
 * Additional Types Definition
 */

/** Slave responses */
typedef enum {
    respNoResponse = 0x00U          /**< no response */
#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE
    , respLinProductID              /**< answer to "Read by Identifier (0)" request */
    , respNegativeResponse          /**< answer to non-supported sub-functions request */
#if SL_EN_SERIAL_NUMBER_CALLOUT
    , respSerialNumber              /**< answer to "Read by Identifier (1)" request */
#endif /* SL_EN_SERIAL_NUMBER_CALLOUT */
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE */
#if SL_vISO17987_2016 && SL_EN_READ_BY_ID_SERVICE
    , respNcfLdfVersion             /**< answer to "Read by Identifier (3)" request */
#endif /* SL_vISO17987_2016 && SL_EN_READ_BY_ID_SERVICE */
#if SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE)
    , respPositiveAssignFID         /**< answer to "Assign frame id" request (LIN2.0) */
#endif /* SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE) */
#if SL_EN_ASSIGN_NAD_SERVICE
    , respPositiveAssignNAD         /**< answer to "Assign NAD" optional request */
#endif /* SL_EN_ASSIGN_NAD_SERVICE */
#if SL_EN_COND_CHANGE_NAD_SERVICE
    , respConditionalChangeNAD      /**< answer to "Conditional change NAD" optional request */
#endif /* SL_EN_COND_CHANGE_NAD_SERVICE */
#if SL_vLIN_2_1_plus || SL_vISO17987_2016
    , respPositiveAssignFidRange    /**< answer to "Assign Frame ID Range" request (LIN2.1, LIN2.2) */
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */
#if (SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE
    , respPositiveSaveConfig        /**< answer to "Save Configuration" request (LIN2.1, LIN2.2) */
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */
#if SL_EN_READ_BY_ID_CALLOUT
    , respUserDefIdCallout          /**< answer to user defined read_by_id request */
#endif /* SL_EN_READ_BY_ID_CALLOUT */
#if SL_vSAE_J2602_2012
    , respPositiveTargetedReset     /**< positive answer to "TargetedReset" */
    , respNegativeTargetedReset     /**< negative answer to "TargetedReset" */
#endif /* SL_vSAE_J2602_2012 */
} SlaveResponse_t;

/** Status word type */
typedef union {
    struct {
        l_bool ErrorInResponse    : 1;    /**< Error in response flag */
        l_bool SuccessfulTrans    : 1;    /**< Successful transfer flag */
        l_bool Overrun            : 1;    /**< Few frames were processed since the previous call */
        l_bool GoToSleep          : 1;    /**< Go to Sleep command was received */
#if SL_vLIN_2_1_plus || SL_vISO17987_2016
        l_bool BusActivity        : 1;    /**< Node has detected activity on the bus */
        l_bool EventTriggered     : 1;    /**< Always 0 (zero) for Slave node */
        l_bool SaveConfig         : 1;    /**< Save configuration request was received */
        l_u8 unused : 1;
#else /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */
        l_u8 unused : 4;
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

        l_u8 PID    : 8;    /**< Protected ID */
    } mapped;

    l_u16 word;             /**< Full ifc status word */

} l_s_IfcStatus_t;

#if SL_vSAE_J2602_2012
/** J2602 reset responses */
typedef enum {
    J2602ResetNoResponse = (l_u8)respNoResponse,             /**< no response */
    J2602ResetPosTarReset = (l_u8)respPositiveTargetedReset, /**< positive answer to "TargetedReset" */
    J2602ResetNegTarReset = (l_u8)respNegativeTargetedReset, /**< negative answer to "TargetedReset" */
} l_J2602ResetResp_t;
#endif /* SL_vSAE_J2602_2012 */

#if SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION
/** NCF/LDF version source */
typedef enum {
    nls_ldf = 1u,  /**< LDF is the version source */
    nls_ncf = 2u   /**< NCF is the version source */
} l_s_NcfLdfSrc_t;

/** NCF/LDF version type */
typedef struct {
    l_u8 major;                 /**< Major version */
    l_u8 minor;                 /**< Minor version */
    l_u8 sub_version;           /**< Sub-version */
    l_s_NcfLdfSrc_t source;     /**< NCF/LDF version */
} l_s_NcfLdfVersion_t;
#endif /* SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION */

#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


/*
 * Function prototypes
 */

/** @name Interface management */
#if SL_vLIN_2_0 || SL_vSAE_J2602_2012 || SL_vLIN_1_3
extern void l_ifc_init_sl1 (void);
extern l_bool l_ifc_connect_sl1 (void);
extern l_bool l_ifc_disconnect_sl1 (void);

#elif SL_vLIN_2_1_plus || SL_vISO17987_2016
extern l_bool l_ifc_init_sl1 (void);
#else
#error "The selected SL_LIN_VERSION isn't supported"
#endif /* SL_vLIN_2_0 || SL_vSAE_J2602_2012 || SL_vLIN_1_3 */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
extern void l_ifc_wake_up_sl1 (void);
extern l_u16 l_ifc_read_status_sl1 (void);
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */
/**@}*/

/** @name Node configuration */
#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
    SL_EN_SAVE_CONFIG_FUNC)
extern l_u8 ld_read_configuration (l_ifc_handle iii, l_u8* const data, l_u8* const length);
extern l_u8 ld_set_configuration (l_ifc_handle iii, const l_u8* const data, l_u16 length);
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */
/**@}*/


#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012

/** @name Identification */
#if SL_EN_READ_BY_ID_CALLOUT
extern l_u8 ld_read_by_id_callout (l_ifc_handle iii, l_u8 id, l_u8* pci, l_u8* data);
#endif /* SL_EN_READ_BY_ID_CALLOUT */
/**@}*/

#if SL_vSAE_J2602_2012 /* J2602 (based on LIN2.0) */
/** @name SAE J2602 API functions and events */
extern l_u8 mlu_SAE_SetStatusApiInfo (ml_MessageID_t midx);
extern void mlu_SAE_MessageReceived (ml_MessageID_t midx);
extern void mlu_SAE_DataTransmitted (void);
extern l_J2602ResetResp_t mlu_SAE_Reset (void);
extern ml_Status_t ml_SAE_DataReady (l_u8 apinfo);
/**@}*/
#endif /* SL_vSAE_J2602_2012 */

/** @name Shared LIN API internals */
#if SL_vLIN_2_x || SL_vISO17987_2016 || (SL_vSAE_J2602_2012 && (SL_EN_ASSIGN_NAD_SERVICE || \
                                                                SL_EN_ASSIGN_FRAME_ID_SERVICE || \
                                                                SL_EN_READ_BY_ID_SERVICE))
extern ml_Status_t isSupplierIdValid (const l_u8 data[]);
extern ml_Status_t isFunctionIdValid (const l_u8 data[]);
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || (SL_vSAE_J2602_2012 && (SL_EN_ASSIGN_NAD_SERVICE || SL_EN_ASSIGN_FRAME_ID_SERVICE || SL_EN_READ_BY_ID_SERVICE)) */
/**@}*/

/** @name Additional helper functions */
extern void l_ifc_sleep_entered_callout (ml_SleepReason_t Reason);
extern void ld_DiagClearPendingResponse(void);

#if (SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012) && SL_EN_SERIAL_NUMBER_CALLOUT
extern void ld_serial_number_callout (l_u8 data[4]);
#endif /* (SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012) && SL_EN_SERIAL_NUMBER_CALLOUT */
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

/** MLX LIN Slave driver initialization.
 * Implementation is platform/project specific.
 * @return void
 */
extern void l_mls_Init(void);
/**@}*/

/** @name LIN Slave driver event handlers */
extern void l_LinSleepMode(ml_SleepReason_t Reason);
extern void l_MessageReceived(ml_MessageID_t MessageIndex);
extern void l_DataRequest (ml_MessageID_t MessageIndex);
extern void l_ErrorDetected(ml_LinError_t Error);
extern void l_DataTransmitted(ml_MessageID_t MessageIndex);
extern void l_AutoAddressingStep(l_u8 StepNumber);
/**@}*/

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
/** @name LIN Slave driver Transport Layer event handlers */
extern bool ld_DiagRequest(LINDiagTransfer_t *transfer);
extern l_u8 ld_GetLINNAD (l_u8 NAD);
/**@}*/
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


/*
 * Externals
 */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012

/** @name Node Address for/by Diagnostics */
extern const l_u8 ml_InitialNAD;
extern l_u8 ml_ConfiguredNAD;
/**@}*/

/** @name LIN Product Identification */
extern const l_u8 ml_ProductID[5];
/**@}*/

#if SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION
/** @name NCF/LDF version */
extern const l_s_NcfLdfVersion_t l_s_NcfLdfVersion;
/**@}*/
#endif /* SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION */

#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

#endif /* LIN_API_SL_H */

/* EOF */
