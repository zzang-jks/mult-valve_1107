/**
 ******************************************************************************
 * @file    lin_cfg_ma.h
 * @brief   LIN Master node configuration and identification API
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
 * @details The LIN node configuration and identification API is request/response (service)
 *          based, i.e. the application in the master node calls an API routine that transmits
 *          a request to the specified slave node and awaits a response.
 *
 * @ingroup lin_master_api
 *
 */

#ifndef LIN_CFG_MA_H
#define LIN_CFG_MA_H

#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif


#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012

/*-----------------------------------------------------------------------------
 * Macros
 */

#define NO_DIAG_RESP_ERROR 0u
#define DEFAULT_RSID_VALUE 0u
#define GET_RSID(sid) ((uint8_t)(sid) | 0x40u)


/*-----------------------------------------------------------------------------
 * Types definition
 */

/** Configuration API statuses */
typedef enum {
    LD_SERVICE_BUSY = 0,    /**< Service is ongoing */
    LD_REQUEST_FINISHED,    /**< The configuration request has been completed.
                                 This is a intermediate status between the configuration request
                                 and configuration response */
    LD_SERVICE_IDLE,        /**< The configuration request/response combination has been completed,
                                 i.e. the response is valid and may be analyzed */
    LD_SERVICE_ERROR        /**< The configuration request or response experienced an error */
} ld_m_CfgApiStatus_t;

/** Configuration API handler for LIN v2.1 and greater */
typedef struct {
    ld_m_CfgApiStatus_t Status;   /**< Configuration API status */
} ld_m_CfgApiHandle_t;

#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
/** Check configuration API response statuses (compatibility mode with LIN v2.0) */
typedef enum {
    CR_LD_SUCCESS = 0,         /**< The call was successfully carried out */
    CR_LD_NEGATIVE,            /**< The call failed, more information can be found by parsing error_code */
    CR_LD_NO_RESPONSE,         /**< No response was received on the request */
    CR_LD_OVERWRITTEN          /**< The slave response frame has been overwritten by another operation,
                                  i.e. the result is lost. This can only occur if the cluster uses both
                                  node configuration and the diagnostic transport layer */
} ld_m_CheckCfgApiResp20_t;
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */

/** Current message processed */
typedef enum {
    ncs_AssignNad            = 0xB0,
#if MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND
    ncs_AssignFrameId        = 0xB1,
#endif /* MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND */
    ncs_ReadById             = 0xB2,
    ncs_ConditChangeNad      = 0xB3,
    ncs_DataDump             = 0xB4,
#if MA_vLIN_2_1_plus || MA_vISO17987_2016
    ncs_SaveConfig           = 0xB6,
    ncs_AssignFrameIdRange   = 0xB7
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */
} ld_m_NodeConfService_t;

/** Diagnostic request data type */
typedef struct {
    ld_m_NodeConfService_t CurrentService;
    l_bool isRespExpected;
} ld_m_DiagRequest_t;

/** Diagnostic response data type */
typedef struct {
    l_bool isPossitive;
    l_u8 RSID;
    l_u8 error;
} ld_m_DiagResponse_t;


/*-----------------------------------------------------------------------------
 * Externals
 */

extern ld_m_DiagRequest_t ld_m_DiagRequest;
extern ld_m_DiagResponse_t ld_m_DiagResponse;
extern ld_m_CfgApiHandle_t ld_m_CfgApiHandler;

#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
extern ld_m_CheckCfgApiResp20_t ld_m_CheckCfgApiResp20;
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */


/*-----------------------------------------------------------------------------
 * Function prototypes
 */

/** @name Diagnostic frames processing */
extern void ld_m_ProcessCfgM2S (l_m_CmdProcEvent_t event);
extern void ld_m_ProcessCfgS2M (l_m_CmdProcEvent_t event);

#if ((MA_vLIN_2_1_plus || MA_vISO17987_2016) && MA_EN_DATA_DUMP_COMMAND)
extern void ld_m_DataDump (l_u8 NAD, l_u8* const data);
#endif /* ((MA_vLIN_2_1_plus || MA_vISO17987_2016) && MA_EN_DATA_DUMP_COMMAND) */

extern void ld_m_FreeFormat (l_u8* const data);
/**@}*/

#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

#endif /* LIN_CFG_MA_H */

/* EOF */
