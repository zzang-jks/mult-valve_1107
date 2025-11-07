/**
 ******************************************************************************
 * @file    lin_cfg_ma.c
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

#include "lin_api.h"

#if LIN_MASTER_API == 1

#include "lin_core_ma.h"
#include "lin_cfg_ma.h"
#include "lin_trans_ma.h"


//TODO: Create the diagram to represent configuration frames processing

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012

/*-----------------------------------------------------------------------------
 * Function prototypes
 */

static void ld_m_ParseResponse (void);


/*-----------------------------------------------------------------------------
 * Variables
 */

ld_m_DiagRequest_t ld_m_DiagRequest;
ld_m_DiagResponse_t ld_m_DiagResponse;
ld_m_CfgApiHandle_t ld_m_CfgApiHandler;

#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
ld_m_CheckCfgApiResp20_t ld_m_CheckCfgApiResp20;
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */

static l_u8 *ld_m_ReadByIdData;


/*-----------------------------------------------------------------------------
 * Functions
 */

#if MA_vLIN_2_1_plus || MA_vISO17987_2016
/** Return the status.
 * This call returns the status of the last requested configuration service (::ld_m_CfgApiStatus_t)
 * @param[in] iii   interface
 * @return  Status
 */
l_u8 ld_is_ready (l_ifc_handle iii)
{
    /* Flash version - unused */
    (void) iii;

    return ( (l_u8)ld_m_CfgApiHandler.Status );
}


/** Return the result.
 * This call returns the result of the last node configuration service, in the parameters
 * RSID and error_code. A value in RSID is always returned but not always in the error_code.
 * Default values for RSID and error_code is 0 (zero).
 * @param[in] iii           interface
 * @param[out] RSID         RSID
 * @param[out] error_code   error code
 * @return  void
 */
void ld_check_response (l_ifc_handle iii, l_u8* const RSID, l_u8* const error_code)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* Flash version - unused */
    (void) iii;

    /* Always provide RSID value */
    *RSID = ld_m_DiagResponse.RSID;
    *error_code = NO_DIAG_RESP_ERROR;

    /* Provide also error_code if Negative response is received */
    if (ld_m_DiagResponse.isPossitive == false) {
        *error_code = ld_m_DiagResponse.error;
    }
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Assign PID of up to four frames.
 * This call assigns the protected identifier of up to four frames in the slave node
 * with the addressed NAD. The PIDs parameter shall be four bytes long, each byte
 * shall contain a PID, do not care or unassign value.
 * @note The function can't be called when ld_is_ready() returns the LD_SERVICE_BUSY status.
 * @param[in] iii           interface
 * @param[in] NAD           configured NAD value
 * @param[in] start_index   start message index
 * @param[in] PIDs          array of four PID values
 * @return  void
 */
void ld_assign_frame_id_range (l_ifc_handle iii, l_u8 NAD, l_u8 start_index, const l_u8* const PIDs)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Flash version - unused */
    (void) iii;

    /* Service parameters */
    const l_u8 PCI = 0x06;
    const l_u8 SID = 0xB7;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = NAD;
    M2S_handler.FrameBuffer[1] = PCI;
    M2S_handler.FrameBuffer[2] = SID;
    M2S_handler.FrameBuffer[3] = start_index;
    M2S_handler.FrameBuffer[4] = PIDs[0];
    M2S_handler.FrameBuffer[5] = PIDs[1];
    M2S_handler.FrameBuffer[6] = PIDs[2];
    M2S_handler.FrameBuffer[7] = PIDs[3];

    M2S_handler.Source.mapped.mrs_Cfg = 1;     /* Set the flag that Configuration frame is requested to be sent */
    ld_m_DiagRequest.isRespExpected = true;
    ld_m_DiagRequest.CurrentService = ncs_AssignFrameIdRange;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Save configuration request.
 * This call will make a save configuration request to a specific slave node with the given NAD,
 * or to all slave nodes if NAD is set to broadcast.
 * @note The function can't be called when ld_is_ready() returns the LD_SERVICE_BUSY status.
 * @param[in] iii       interface
 * @param[in] NAD       configured NAD value
 * @return  void
 */
void ld_save_configuration (l_ifc_handle iii, l_u8 NAD)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Flash version - unused */
    (void) iii;

    /* Service parameters */
    const l_u8 PCI = 0x01;
    const l_u8 SID = 0xB6;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = NAD;
    M2S_handler.FrameBuffer[1] = PCI;
    M2S_handler.FrameBuffer[2] = SID;
    M2S_handler.FrameBuffer[3] = 0xFF;     /* unused */
    M2S_handler.FrameBuffer[4] = 0xFF;     /* unused */
    M2S_handler.FrameBuffer[5] = 0xFF;     /* unused */
    M2S_handler.FrameBuffer[6] = 0xFF;     /* unused */
    M2S_handler.FrameBuffer[7] = 0xFF;     /* unused */

    M2S_handler.Source.mapped.mrs_Cfg = 1;     /* Set the flag that Configuration frame is requested to be sent */
    ld_m_DiagRequest.isRespExpected = true;
    ld_m_DiagRequest.CurrentService = ncs_SaveConfig;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */


/** Assign NAD request.
 * This call assigns the NAD (node diagnostic address) of all slave nodes that matches the initial_NAD,
 * the supplier ID and the function ID. The new NAD of the slave node will be new_NAD.
 * @note The function can't be called when ld_is_ready() returns the LD_SERVICE_BUSY status.
 * @param[in] iii           interface
 * @param[in] initial_NAD   initial NAD value
 * @param[in] supplier_id   supplier ID value
 * @param[in] function_id   functional ID value
 * @param[in] new_NAD       new configured NAD
 * @return  void
 */
void ld_assign_NAD (l_ifc_handle iii, l_u8 initial_NAD, l_u16 supplier_id, l_u16 function_id, l_u8 new_NAD)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Flash version - unused */
    (void) iii;

    /* Service parameters */
    const l_u8 PCI = 0x06;
    const l_u8 SID = 0xB0;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = initial_NAD;
    M2S_handler.FrameBuffer[1] = PCI;
    M2S_handler.FrameBuffer[2] = SID;
    M2S_handler.FrameBuffer[3] = (l_u8)(supplier_id & 0xFFu);            /* LSB */
    M2S_handler.FrameBuffer[4] = (l_u8)((supplier_id >> 8) & 0xFFu);     /* MSB */
    M2S_handler.FrameBuffer[5] = (l_u8)(function_id & 0xFFu);            /* LSB */
    M2S_handler.FrameBuffer[6] = (l_u8)((function_id >> 8) & 0xFFu);     /* MSB */
    M2S_handler.FrameBuffer[7] = new_NAD;

    M2S_handler.Source.mapped.mrs_Cfg = 1;     /* Set the flag that Configuration frame is requested to be sent */
    ld_m_DiagRequest.isRespExpected = true;
    ld_m_DiagRequest.CurrentService = ncs_AssignNad;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Conditional NAD change.
 * This call changes the NAD if the node properties fulfil the test specified by id, byte, mask and invert.
 * @note The function can't be called when ld_is_ready() returns the LD_SERVICE_BUSY status.
 * @param[in] iii       interface
 * @param[in] NAD       configured NAD value
 * @param[in] id        ID of the data structure which is used for read by response
 * @param[in] byte      byte number [1..5] into the data structure specified by ID
 * @param[in] mask      mask value used for bitwise AND operation over the data selected by byte number
 * @param[in] invert    invert value used for bitwise XOR operation over the data selected by byte number
 * @param[in] new_NAD   new configured NAD value
 * @return  void
 */
void ld_conditional_change_NAD (l_ifc_handle iii, l_u8 NAD, l_u8 id, l_u8 byte, l_u8 mask, l_u8 invert, l_u8 new_NAD)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Flash version - unused */
    (void) iii;

    /* Service parameters */
    const l_u8 PCI = 0x06;
    const l_u8 SID = 0xB3;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = NAD;
    M2S_handler.FrameBuffer[1] = PCI;
    M2S_handler.FrameBuffer[2] = SID;
    M2S_handler.FrameBuffer[3] = id;
    M2S_handler.FrameBuffer[4] = byte;
    M2S_handler.FrameBuffer[5] = mask;
    M2S_handler.FrameBuffer[6] = invert;
    M2S_handler.FrameBuffer[7] = new_NAD;

    M2S_handler.Source.mapped.mrs_Cfg = 1;     /* Set the flag that Configuration frame is requested to be sent */
    ld_m_DiagRequest.isRespExpected = true;
    ld_m_DiagRequest.CurrentService = ncs_ConditChangeNad;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Read by identifier request.
 * The call requests the slave node selected with the NAD to return the property associated with the id parameter.
 * @note The function can't be called when ld_is_ready() returns the LD_SERVICE_BUSY status.
 * @param[in] iii           interface
 * @param[in] NAD           configured NAD value
 * @param[in] supplier_id   supplier ID value
 * @param[in] function_id   functional ID value
 * @param[in] id            ID of the data structure
 * @param[out] data
 * @return  void
 */
void ld_read_by_id (l_ifc_handle iii, l_u8 NAD, l_u16 supplier_id, l_u16 function_id, l_u8 id, l_u8* const data)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Flash version - unused */
    (void) iii;

    /* Service parameters */
    const l_u8 PCI = 0x06;
    const l_u8 SID = 0xB2;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = NAD;
    M2S_handler.FrameBuffer[1] = PCI;
    M2S_handler.FrameBuffer[2] = SID;
    M2S_handler.FrameBuffer[3] = id;
    M2S_handler.FrameBuffer[4] = (l_u8)(supplier_id & 0xFFu);            /* LSB */
    M2S_handler.FrameBuffer[5] = (l_u8)((supplier_id >> 8) & 0xFFu);     /* MSB */
    M2S_handler.FrameBuffer[6] = (l_u8)(function_id & 0xFFu);            /* LSB */
    M2S_handler.FrameBuffer[7] = (l_u8)((function_id >> 8) & 0xFFu);     /* MSB */

    M2S_handler.Source.mapped.mrs_Cfg = 1;     /* Set the flag that Configuration frame is requested to be sent */

    /* Set Read By Identifier processing flag */
    ld_m_DiagRequest.isRespExpected = true;
    ld_m_DiagRequest.CurrentService = ncs_ReadById;
    ld_m_ReadByIdData = data;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


#if MA_vLIN_2_0 || MA_vSAE_J2602_2012

/** Return the status.
 * This call returns the status of the last requested configuration service (::ld_m_CfgApiStatus_t)
 * @param[in] iii   interface
 * @return  Status
 */
l_bool ld_is_ready (l_ifc_handle iii)
{
    /* Flash version - unused */
    (void) iii;

    l_bool retVal = (ld_m_CfgApiHandler.Status == LD_SERVICE_IDLE)
                    || (ld_m_CfgApiHandler.Status == LD_SERVICE_ERROR);
    return (retVal);
}


/** Return the result.
 * This call returns the result of the last node configuration service, in the parameters
 * RSID and error_code. A value in RSID is always returned but not always in the error_code.
 * Default values for RSID and error_code is 0 (zero).
 * @param[in] iii           interface
 * @param[out] RSID         RSID
 * @param[out] error_code   error code
 * @return  ::ld_m_CheckCfgApiResp20_t
 */
l_u8 ld_check_response (l_ifc_handle iii, l_u8* RSID, l_u8* error_code)
{
    /* Flash version - unused */
    (void) iii;

    /* Always provide RSID value */
    *RSID = ld_m_DiagResponse.RSID;
    *error_code = NO_DIAG_RESP_ERROR;

    /* Provide also error_code if Negative response is received */
    if (ld_m_DiagResponse.isPossitive == false) {
        *error_code = ld_m_DiagResponse.error;
    }

    return ( (l_u8)ld_m_CheckCfgApiResp20 );
}
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */


#if MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND
/** Assign PID for one frame.
 * This call assigns the protected identifier of a frame in the slave node with the address NAD
 * and the specified supplier ID. The frame changed shall have the specified message ID
 * and will after the call have PID as the the protected identifier.
 * @note The function can't be called when ld_is_ready() returns the LD_SERVICE_BUSY status.
 * @param[in] iii           interface
 * @param[in] NAD           configured NAD value
 * @param[in] supplier_id   supplier ID
 * @param[in] message_id    message ID to assign
 * @param[in] PID           PID value to assign
 * @return  void
 */
void ld_assign_frame_id (l_ifc_handle iii, l_u8 NAD, l_u16 supplier_id, l_u16 message_id, l_u8 PID)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Flash version - unused */
    (void) iii;

    /* Service parameters */
    const l_u8 PCI = 0x06;
    const l_u8 SID = 0xB1;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = NAD;
    M2S_handler.FrameBuffer[1] = PCI;
    M2S_handler.FrameBuffer[2] = SID;
    M2S_handler.FrameBuffer[3] = (l_u8)(supplier_id & 0xFFu);            /* LSB */
    M2S_handler.FrameBuffer[4] = (l_u8)((supplier_id >> 8) & 0xFFu);     /* MSB */
    M2S_handler.FrameBuffer[5] = (l_u8)(message_id & 0xFFu);             /* LSB */
    M2S_handler.FrameBuffer[6] = (l_u8)((message_id >> 8) & 0xFFu);      /* MSB */
    M2S_handler.FrameBuffer[7] = PID;

    M2S_handler.Source.mapped.mrs_Cfg = 1;     /* Set the flag that Configuration frame is requested to be sent */
    ld_m_DiagRequest.isRespExpected = true;
    ld_m_DiagRequest.CurrentService = ncs_AssignFrameId;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}
#endif /* MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND */


#if ((MA_vLIN_2_1_plus || MA_vISO17987_2016) && MA_EN_DATA_DUMP_COMMAND)
/** Generates a data dump request.
 * Five data bytes will be transmitted to Slave.
 * @param[in] NAD
 * @param[in] data   array of 5 bytes
 * @return  void
 */
void ld_m_DataDump (l_u8 NAD, l_u8* const data)
{
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Service parameters */
    const l_u8 PCI = 0x06;
    const l_u8 SID = 0xB4;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = NAD;
    M2S_handler.FrameBuffer[1] = PCI;
    M2S_handler.FrameBuffer[2] = SID;
    M2S_handler.FrameBuffer[3] = data[0];
    M2S_handler.FrameBuffer[4] = data[1];
    M2S_handler.FrameBuffer[5] = data[2];
    M2S_handler.FrameBuffer[6] = data[3];
    M2S_handler.FrameBuffer[7] = data[4];

    M2S_handler.Source.mapped.mrs_Cfg = 1; /* Set the flag that Configuration frame is requested to be sent */
    ld_m_DiagRequest.isRespExpected = true;
    ld_m_DiagRequest.CurrentService = ncs_DataDump;
}
#endif /* ((MA_vLIN_2_1_plus || MA_vISO17987_2016) && MA_EN_DATA_DUMP_COMMAND) */


#if MA_EN_FREE_FORMAT_COMMAND
/** Free data transmit request.
 * FreeFormat transmits a fixed master request frame with the eight data bytes provided.
 * @param[in] data   array of eight bytes
 * @return  void
 */
void ld_m_FreeFormat (l_u8* const data)
{
    /* The configuration API is considered BUSY at the very beginning of the call processing.
     * It's blocking state for other configuration API function calls.
     * The Status shall be checked before other configuration API calls with ld_is_ready() */
    ld_m_CfgApiHandler.Status = LD_SERVICE_BUSY;

    /* Fill the M2S frame buffer */
    M2S_handler.FrameBuffer[0] = data[0];
    M2S_handler.FrameBuffer[1] = data[1];
    M2S_handler.FrameBuffer[2] = data[2];
    M2S_handler.FrameBuffer[3] = data[3];
    M2S_handler.FrameBuffer[4] = data[4];
    M2S_handler.FrameBuffer[5] = data[5];
    M2S_handler.FrameBuffer[6] = data[6];
    M2S_handler.FrameBuffer[7] = data[7];

    M2S_handler.Source.mapped.mrs_Cfg = 1; /* Set the flag that Configuration frame is requested to be sent */
    ld_m_DiagRequest.isRespExpected = false;
}
#endif /* MA_EN_FREE_FORMAT_COMMAND */


/** Process diagnostic request frame.
 * This function process master request frame for Configuration and Identification API.
 * The function includes Status management functionality.
 * @param[in] event   event to be handled
 * @return  void
 */
void ld_m_ProcessCfgM2S (l_m_CmdProcEvent_t event)
{
    switch (event) {
        case cpe_FrameSlotStart: /* The data already should be prepared */
        {
            /* Fill the structure for LIN Master driver */ //TODO: Shared memory!
            l_m_Driver.Frame.PID = 0x3C;
            l_m_Driver.Frame.ChecksumType = Classic;
            l_m_Driver.Frame.DataLen = 8;
            l_m_Driver.Frame.Data = M2S_handler.FrameBuffer; /* Should be a pointer to reserved memory area */
            l_m_Driver.Frame.FrameType = M2S;

            /* Process the Frame; ignore the returned value */

            /* TODO: Deliver an error on transmit to interface handler if failed */
            if (lin_master_HandleLinFrame(&l_m_Driver.Frame) == 1u) {
                S2M_handler.Target.mapped.srt_Cfg = 1; /* Slave response referred to Node configuration and identification API is awaited */
            } else {
                ld_m_CfgApiHandler.Status = LD_SERVICE_ERROR;
                l_m_IfcStatusHelper.isFrameError = true; /* Set flag for ifc status */
            }
            break;
        }
        case cpe_FrameError:
            ld_m_CfgApiHandler.Status = LD_SERVICE_ERROR;
            l_m_IfcStatusHelper.isFrameError = true; /* Set flag for ifc status */
            ld_m_DiagRequest.isRespExpected = false; /* No response expected */
            break;
        case cpe_FrameTransmitted:
            M2S_handler.Source.mapped.mrs_Cfg = 0; /* Reset Configuration message for M2S */
            ld_m_CfgApiHandler.Status = LD_REQUEST_FINISHED;
            break;
        default:
            /* Process another events individually for each interface */
            break;
    }
}


/** Process diagnostic response frame.
 * This function process slave response frame for Configuration and Identification API.
 * The function includes Status management functionality.
 * @param[in] event   event to be handled
 * @return  void
 */
void ld_m_ProcessCfgS2M (l_m_CmdProcEvent_t event)
{
    switch (event) {
        case cpe_FrameSlotStart:
        {
            /* Fill the structure for LIN Master driver */ //TODO: Shared memory!
            l_m_Driver.Frame.PID = 0x7D;
            l_m_Driver.Frame.ChecksumType = Classic;
            l_m_Driver.Frame.DataLen = 8;
            l_m_Driver.Frame.Data = S2M_handler.FrameBuffer;
            l_m_Driver.Frame.FrameType = S2M;

            /* Process the Frame; ignore the returned value */
            if (lin_master_HandleLinFrame(&l_m_Driver.Frame) == 1u) {
                /* keeping ld_m_CfgApiHandler.Status = LD_REQUEST_FINISHED */
            } else {   /* Driver can't start the frame processing; Driver is busy */
                ld_m_CfgApiHandler.Status = LD_SERVICE_ERROR;
                l_m_IfcStatusHelper.isFrameError = true; /* Set flag for ifc status */
            }
            break;
        }
        case cpe_RxCollision:
        case cpe_FrameError:
        case cpe_NoResponse:
            ld_m_CfgApiHandler.Status = LD_SERVICE_ERROR;
#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
            /* Note: errors are counted as LD_NO_RESPONSE */
            ld_m_CheckCfgApiResp20 = CR_LD_NO_RESPONSE;
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */
            l_m_IfcStatusHelper.isFrameError = true; /* Set flag for ifc status */
            ld_m_DiagRequest.isRespExpected = false; /* TODO: Recheck: the response retry isn't supported */
            break;
        case cpe_FrameReceived:
            /* Note: If S2M_handler was set by another command before the end of this frame receiving, then new frame will be reseted (ignored) */
            S2M_handler.Target.mapped.srt_Cfg = 0;   /* Reset Configuration message for S2M */
            ld_m_ParseResponse();                    /* Provide the output for ld_check_response() */
            ld_m_CfgApiHandler.Status = LD_SERVICE_IDLE;
            ld_m_DiagRequest.isRespExpected = false; /* Response expected only once */
            break;
        default:
            /* There is no action supported */
            break;
    }
}


/** Diagnostic response frame parsing.
 * Slave response parsing for ld_check_response functionality.
 * @return  void
 */
static void ld_m_ParseResponse (void)
{
    if (ld_m_DiagRequest.isRespExpected == true) {
        l_u8 rec_RSID = l_m_Driver.Frame.Data[2];                   /* Received RSID */
        l_u8 exp_RSID = GET_RSID(ld_m_DiagRequest.CurrentService);  /* Expected RSID */

        /* If received diag response is the same as expected -> positive response; negative response otherwise */
        if (rec_RSID == exp_RSID) { /* Positive response */
            ENTER_STD_LIN_API_ATOMIC_SECTION();
            ld_m_DiagResponse.isPossitive = true;
            ld_m_DiagResponse.RSID = rec_RSID;
#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
            ld_m_CheckCfgApiResp20 = CR_LD_SUCCESS;
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */
            EXIT_STD_LIN_API_ATOMIC_SECTION();
        } else {   /* Negative response */
            ENTER_STD_LIN_API_ATOMIC_SECTION();
            ld_m_DiagResponse.isPossitive = false;
            ld_m_DiagResponse.RSID = l_m_Driver.Frame.Data[3];      /* Requested SID */
            ld_m_DiagResponse.error = l_m_Driver.Frame.Data[4];     /* Error code */
#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
            ld_m_CheckCfgApiResp20 = CR_LD_NEGATIVE;
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */
            EXIT_STD_LIN_API_ATOMIC_SECTION();
        }

        /* Service specific actions */
        if (ncs_ReadById == ld_m_DiagRequest.CurrentService) {
            l_u8 cnt_op = 7; /* Eight bit position */
            for (l_u8 cnt = 0; cnt < l_m_Driver.Frame.Data[1]; cnt++) {
                /* Copy data bytes in a big-endian style */
                ld_m_ReadByIdData[cnt] = l_m_Driver.Frame.Data[cnt_op];
                cnt_op--;
            }
        } else {   /* Nothing for another services */
        }
    }
}
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

#endif /* LIN_MASTER_API == 1 */
/* EOF */
