/**
 ******************************************************************************
 * @file    lin_cfg_sl.c
 * @brief   LIN Slave node configuration and identification API
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
 * @details The LIN node configuration and identification API.
 *
 * @ingroup lin_slave_api
 *
 */

#include "lin_api.h"

#if LIN_SLAVE_API == 1

#include "lin_core.h"

/* Interface specific API */
#include "lin_cfg_sl.h"
#include "lin_core_sl.h"


/*-----------------------------------------------------------------------------
 * Macro
 */

#define SLAVE_RESPONSE_FRAME_INITIALIZER    {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}


/*-----------------------------------------------------------------------------
 * Function prototypes
 */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012

static bool ld_DiagResponse (void);

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE
static SlaveResponse_t readByIdentifier (void);
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE */

#if SL_EN_ASSIGN_NAD_SERVICE
static SlaveResponse_t assignNAD (void);
#endif /* SL_EN_ASSIGN_NAD_SERVICE */

#if SL_EN_COND_CHANGE_NAD_SERVICE
static SlaveResponse_t conditionalChangeNAD (void);
#endif /* SL_EN_COND_CHANGE_NAD_SERVICE */

#if SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE)
static SlaveResponse_t assignFrameID (void);
#endif /* SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE) */

#if SL_vLIN_2_1_plus || SL_vISO17987_2016
static SlaveResponse_t assignFrameIdRange (void);
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

#if (SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE
static SlaveResponse_t saveConfiguration (void);
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
    SL_EN_SAVE_CONFIG_FUNC)
static l_u8 ld_set_configuration_helper (const l_u8* const data, l_u16 length);
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */

#if SL_EN_READ_BY_ID_CALLOUT
__attribute__((weak)) l_u8 ld_read_by_id_callout (l_ifc_handle iii, l_u8 id, l_u8* pci, l_u8* data);
#endif /* SL_EN_READ_BY_ID_CALLOUT */

#if SL_EN_SERIAL_NUMBER_CALLOUT
__attribute__((weak)) void ld_serial_number_callout (l_u8 data[4]);
#endif /* SL_EN_SERIAL_NUMBER_CALLOUT */

#if SL_vSAE_J2602_2012
__attribute__((weak)) void mlu_SAE_MessageReceived (ml_MessageID_t midx);
__attribute__((weak)) void mlu_SAE_DataTransmitted (void);
#endif /* SL_vSAE_J2602_2012 */


/*-----------------------------------------------------------------------------
 * Variables
 */

/*
 * Type of response that should be sent upon receiving 0x3D (SRF) request.
 * Additional non-diagnostic frames can be sent between 0x3C (MRF) and
 * 0x3D(SRF)
 */
static volatile SlaveResponse_t pendingSlaveResponse;
static LINDiagTransfer_t *pDiagTransfer;


/*-----------------------------------------------------------------------------
 * Functions
 */

#if SL_vSAE_J2602_2012
/** Extension of the ml_DataReady function for J2602 LIN-API
 * It inserts SAE J2602 Status byte as Data Byte 0 in ML_SLAVE_FRAME_DATA_BUFFER and
 * sends ml_DataReady command.
 * @param[in]   apinfo     5-bit of application-dependent information
 * @return Status:
 * - ML_SUCCESS  There is data to send;
 * - ML_FAILURE  There is nothing to send (no response).
 */
ml_Status_t ml_SAE_DataReady (l_u8 apinfo)
{
    l_u8 bitcnt;

    /* Find the last (most significant) bit set i.e. highest prio error */
    bitcnt = (l_u8)STD_LIN_FSB(s_J2602Status.J2602Error.J2602ErrorByte);

    if (bitcnt == 0u) {
        ((l_u8*)ML_SLAVE_FRAME_DATA_BUFFER)[0] = apinfo & 0x1Fu;
    } else {
        s_J2602Status.J2602CurrentError = bitcnt; /* current Error state to report */
        /* insert SAE J2602 Status byte as a first byte of the frame */
        ((l_u8*)ML_SLAVE_FRAME_DATA_BUFFER)[0] = (s_J2602Status.J2602CurrentError << 5) | (apinfo & 0x1Fu);
        s_J2602Status.J2602HasStatusFlag = 1;    /* mark the frame */
    }

    /* The DataTransmitted event is ENABLED for this frame */
    return ml_DataReady(ML_END_OF_TX_ENABLED);
}
#endif /* SL_vSAE_J2602_2012 */

#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
    SL_EN_SAVE_CONFIG_FUNC)

/** Read the node configuration.
 * The function will serialize the current configuration (NAD, PIDs) and copy it to the area (data pointer)
 * provided by the application. The intention is to call this function when the save configuration request
 * flag is set in the interface status word, whereas the application is responsible to save this data
 * in non-volatile memory.
 * @note The function call shall not interrupt the LIN Slave ISR.
 * @param[in]   iii     ifc handler
 * @param[out]  data    pointer to the data buffer provided by application
 * @param[out]  length  pointer to the length of the buffer
 * @return  Status:
 * - LD_READ_OK             - if the service was successful;
 * - LD_LENGTH_TOO_SHORT    - if the configuration size is greater than the length.
 */
l_u8 ld_read_configuration (l_ifc_handle iii, l_u8* const data, l_u8* const length)
{
    /* Flash version - ignore */
    (void) iii;

    ENTER_STD_LIN_API_ATOMIC_SECTION();

    uint_fast8_t i;     /* iterator */
    l_u8 retVal;

    if (*length < (SL_NUMBER_OF_DYNAMIC_MESSAGES + 1U)) {
        /* returns proper length to the application */
        *length = SL_NUMBER_OF_DYNAMIC_MESSAGES + 1U;
        retVal = (l_u8)LD_LENGTH_TOO_SHORT;
    } else {
#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
        data[0] = ml_ConfiguredNAD;
#else
        data[0] = 0;
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

        /* other data filled in by assignFrameIdRange */
        for (i = 0U; i < SL_NUMBER_OF_DYNAMIC_MESSAGES; i++) {
            data[i+1U] = node_configuration[i];
        }

        /* returns actual length to the application */
        *length = SL_NUMBER_OF_DYNAMIC_MESSAGES + 1U;
        retVal = (l_u8)LD_READ_OK;
    }
    return retVal;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Set the node configuration.
 * The function will configure the NAD and the PIDs according to the configuration
 * given by data. The intended usage is to restore a saved configuration or set
 * an initial configuration (e.g. coded by I/O pins).
 * @param[in]  iii     ifc handler
 * @param[in]  data    pointer to the data buffer from application
 * @param[in]  length  pointer to the length of the buffer
 * @return  Status:
 * - LD_READ_OK             - if the service was successful;
 * - LD_LENGTH_NOT_CORRECT  - if the required size of the configuration is not equal to the given length;
 * - LD_DATA_ERROR          - the set of configuration could not be made.
 * @note This function shall NOT be used in other interrupts which can interrupt LIN ISR.
 */
l_u8 ld_set_configuration (l_ifc_handle iii, const l_u8* const data, l_u16 length)
{
    /* Flash version - ignore */
    (void) iii;

    ENTER_STD_LIN_API_ATOMIC_SECTION();
    l_u8 retVal = ld_set_configuration_helper (data, length);
    return retVal;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** The helper functions to set the node configuration.
 * The function will configure the NAD and the PIDs according to the configuration
 * given by data. The intended usage is to restore a saved configuration or set
 * an initial configuration (e.g. coded by I/O pins).
 * @param[in]  data    pointer to the data buffer from application
 * @param[in]  length  pointer to the length of the buffer
 * @return  Status:
 * - LD_READ_OK             - if the service was successful;
 * - LD_LENGTH_NOT_CORRECT  - if the required size of the configuration is not equal to the given length;
 * - LD_DATA_ERROR          - the set of configuration could not be made.
 * @note This function shall NOT be used in other interrupts which can interrupt LIN ISR.
 * @note The ld_set_configuration_helper() changes the same variables and uses the same hardware as
 * the "Assign frame identifier range" frame processing functionality. Race condition is possible
 * in that case.
 */
static l_u8 ld_set_configuration_helper (const l_u8* const data, l_u16 length)
{
    uint_fast8_t i; /* iterator */

    if ((SL_NUMBER_OF_DYNAMIC_MESSAGES + 1U) != length) {
        return (l_u8)LD_LENGTH_NOT_CORRECT;
    }
    /* else: length is Ok, continue processing .. */


    for (i = 0U; i < SL_NUMBER_OF_DYNAMIC_MESSAGES; i++) {
        if (data[i+1U] == 0U) { /* unassign this message index */
            (void)ml_Disconnect();
            if (ml_DisableMessage((ml_MessageID_t)i) != ML_SUCCESS) {
                (void)ml_Connect();
                return (l_u8)LD_DATA_ERROR;
            } else {
                /* it's successful: do nothing */
            }
            (void)ml_Connect();
        } else if (data[i+1U] != 0xFFU) {   /* 0xFF is "don't care" value */
            (void)ml_Disconnect();
            if (ml_AssignFrameToMessageID((ml_MessageID_t)i, data[i+1U]) != ML_SUCCESS) {
                (void)ml_Connect();
                return (l_u8)LD_DATA_ERROR; /* configuration could not be set */
            } else {
                /* it's successful: do nothing */
            }
            (void)ml_Connect();
        } else {
            /* no other special id, continue processing .. */
        }
    }

    /* if no return occurred, new configuration can be stored, otherwise the previous one remains */
    for (i = 0U; i < SL_NUMBER_OF_DYNAMIC_MESSAGES; i++) {
        node_configuration[i] = data[i+1U];

#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
        /*
         * For Associated Unconditional Frames update PID
         */
        if (sft_UncondAssociatedFrame == frameList[i].FrameType) {
            l_s_AssociatedUncondFrame_t *assocFrame = (l_s_AssociatedUncondFrame_t*)frameList[i].Frame;
            assocFrame->PID = data[i+1U];
        }
        /* else: don't store PID */
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */
    }

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
    ml_ConfiguredNAD = data[0];
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

    return (l_u8)LD_SET_OK;
}
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */


#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012

#if SL_EN_READ_BY_ID_CALLOUT
/** User defined read-by-identifier request.
 * This callout is used when the master node transmits a read by identifier request with
 * an identifier in the user defined area. The slave node application will be called from
 * the driver when such request is received.
 *
 * This callout function is called by the std LIN API when a Read by identifier request
 * with an identifier in the range of 32 to 63 inclusive is received from the master.
 *
 * The data buffer is preinitialised with recessive values (0xFF), so, it's not needed to be fully filled in
 * by the application.
 *
 * This callout function may also be called by the std LIN API in case an identifier
 * in this range is used in Conditional Change NAD request received from the master.
 * @note The driver will act according to the return values from the application.
 * @note This service is optional in the network and the callout is available in slave node only.
 * In case the user defined read by identifier request is used, the slave node application must implement this callout.
 * @param[in]   iii     ifc handler
 * @param[in]   id      identifier in the user defined area (32 to 63)
 * @param[in]   pci     pointer for setting PCI of response
 * @param[in]   data    pointer to response buffer
 * @return  Status:
 * - LD_NEGATIVE_RESPONSE   - the slave node will respond with a negative response;
 * - LD_POSITIVE_RESPONSE   - the slave node will setup a positive response;
 * - LD_NO_RESPONSE         - the slave node will not answer.
 */
l_u8 ld_read_by_id_callout (l_ifc_handle iii, l_u8 id, l_u8* pci, l_u8* data)
{
    /*
     * No realisation by default (should be implemented on application side)
     */

    /* Flash version - ignore */
    (void) iii;
    (void) id;
    (void) pci;
    (void) data;
    return 0u;
}
#endif /* SL_EN_READ_BY_ID_CALLOUT */


#if SL_EN_SERIAL_NUMBER_CALLOUT
/** Call out from std LIN API to get Serial Number of the device.
 * This callout function is called by the std LIN API to retrieve the serial number from the user application.
 * The serial number is requested by the std LIN API when preparing a response to master Read by identifier
 * request with id=1 (serial number).
 * @param[in]   data    pointer to response buffer
 * @return      void
 */
void ld_serial_number_callout (l_u8 data[4])
{
    /*
     * No realisation by default (should be implemented on application side)
     */

    (void)data;
}
#endif /* SL_EN_SERIAL_NUMBER_CALLOUT */


#if SL_vSAE_J2602_2012
/** Call out from LIN driver to make an action on the frame receiving.
 * The slave node application will be called from the driver when message is received.
 * J2602 LIN-API event: User Application message has been received
 * @param[in]   midx    message-index of the received message
 * @return      void
 */
void mlu_SAE_MessageReceived (ml_MessageID_t midx)
{
    /* Nothing to do by default */
    (void)midx;
}


/** Call out from LIN driver to make an action on the frame successful transmitting.
 * he slave node application will be called from the driver when frame is transmitted.
 * @return      void
 */
void mlu_SAE_DataTransmitted (void)
{
    /* Nothing to do by default */
}
#endif /* SL_vSAE_J2602_2012 */


/** (Re)initialize the configuration/identification layer
 * @return  void
 */
void ld_DiagInit (void)
{
    pendingSlaveResponse = respNoResponse;
    ml_ConfiguredNAD = ml_InitialNAD;
}


/** The function is called when Master Request Frame (0x3C) is received.
 * @param[in] transfer Request buffer with the transmitted data.
 * @return Status:
 * - true  There is data to send;
 * - false There is nothing to send (no response).
 */
bool ld_DiagRequest(LINDiagTransfer_t *transfer)
{
    pDiagTransfer = transfer;

    /* A new MRF received,
     * so reset pending response
     */
    pendingSlaveResponse = respNoResponse;

    switch (pDiagTransfer->request.reqSId) {

#if SL_EN_ASSIGN_NAD_SERVICE
        case 0xB0: /* Assign NAD request */
            pendingSlaveResponse = assignNAD();
            break;
#endif /* SL_EN_ASSIGN_NAD_SERVICE */

#if SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE)
        case 0xB1: /* Assign FrameID */
            pendingSlaveResponse = assignFrameID();
            break;
#endif /* SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE) */

#if SL_vLIN_2_x || SL_vISO17987_2016 || (SL_vSAE_J2602_2012 && SL_EN_READ_BY_ID_SERVICE)
        case 0xB2: /* Read by Identifier request */
            pendingSlaveResponse = readByIdentifier();
            break;
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || (SL_vSAE_J2602_2012 && SL_EN_READ_BY_ID_SERVICE) */

#if SL_EN_COND_CHANGE_NAD_SERVICE
        case 0xB3: /* Conditional change NAD request (LIN2.x) */
            pendingSlaveResponse = conditionalChangeNAD();
            break;
#endif /* SL_EN_COND_CHANGE_NAD_SERVICE */

#if (SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE
        case 0xB6: /* Save Configuration request */
            pendingSlaveResponse = saveConfiguration();
            break; /* !case 0xB6 */
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

#if SL_vLIN_2_1_plus || SL_vISO17987_2016
        case 0xB7: /* Assign Frame ID range */
            pendingSlaveResponse = assignFrameIdRange();
            break; /* !case 0xB7 */
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

#if SL_vSAE_J2602_2012
        case 0xB5: /* SAE J2602: Targeted/Broadcast Reset request 3C NAD 01 B5 FF FF FF FF FF */
            if (pDiagTransfer->request.dataLen == 0x00u) { /* check PCI-1 */
                pendingSlaveResponse = (SlaveResponse_t)mlu_SAE_Reset();
                s_J2602Status.J2602Error.J2602ErrorByte = 0; /* reset all errors */
                s_J2602Status.J2602Error.mapped.flagReset = 1;
            }
            /* else : If PCI != 0x01 - nothing to do */
            break; /* !case 0xB */
#endif /* SL_vSAE_J2602_2012 */

        default:
            /* There is no action supported */
            break;
    }

    /* Prepare the response */
    return ld_DiagResponse();
}


/** The function is called when Slave Response Frame (0x3D) is requested.
 * @return Status:
 * - true  There is data to send;
 * - false There is nothing to send (no response).
 */
static bool ld_DiagResponse (void)
{
    bool retVal = true;
    SlaveResponse_t thisPendingSlaveResponse = pendingSlaveResponse;

    /* reset pending response after processing */
    /* speculative next state/response, overwrite if otherwise */
    pendingSlaveResponse = respNoResponse;

    switch (thisPendingSlaveResponse) {
        case respNoResponse:
            retVal = false;
            break;

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE
        case respLinProductID:
            pDiagTransfer->response.respSId = RSID(SID_ReadById);
            pDiagTransfer->response.dataLen = 5u; /* Only data bytes length */
            pDiagTransfer->response.data[0] = ml_ProductID[0];
            pDiagTransfer->response.data[1] = ml_ProductID[1];
            pDiagTransfer->response.data[2] = ml_ProductID[2];
            pDiagTransfer->response.data[3] = ml_ProductID[3];
            pDiagTransfer->response.data[4] = ml_ProductID[4];
            break;

#if SL_EN_SERIAL_NUMBER_CALLOUT
        case respSerialNumber:
            pDiagTransfer->response.respSId = RSID(SID_ReadById);
            pDiagTransfer->response.dataLen = 4u; /* Only data bytes length */
            ld_serial_number_callout(&pDiagTransfer->response.data[0]); /* Insert Serial Number into data bytes 3..6 */
            break;
#endif /* SL_EN_SERIAL_NUMBER_CALLOUT */

#if SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION
        case respNcfLdfVersion:
            pDiagTransfer->response.respSId = RSID(SID_ReadById);
            pDiagTransfer->response.dataLen = 5u; /* Only data bytes length */
            pDiagTransfer->response.data[0] = l_s_NcfLdfVersion.major;
            pDiagTransfer->response.data[1] = l_s_NcfLdfVersion.minor;
            pDiagTransfer->response.data[2] = l_s_NcfLdfVersion.sub_version;
            pDiagTransfer->response.data[3] = l_s_NcfLdfVersion.source;
            pDiagTransfer->response.data[4] = 0x00u; /* reserved */
            break;
#endif /* SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION */

        case respNegativeResponse:
            pDiagTransfer->response.respSId = RSID_NegativeResponse;
            pDiagTransfer->response.dataLen = 2u; /* Only data bytes length */
            pDiagTransfer->response.data[0] = 0xB2u;  /* Data1: Requested ID */
            pDiagTransfer->response.data[1] = 0x12u;  /* Data2: Error code */
            break;
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE */

#if SL_EN_ASSIGN_NAD_SERVICE
        case respPositiveAssignNAD:
            pDiagTransfer->response.respSId = RSID(SID_AssignNad);
            pDiagTransfer->response.dataLen = 0u; /* Only data bytes length */
            break;
#endif /* SL_EN_ASSIGN_NAD_SERVICE */

#if SL_EN_COND_CHANGE_NAD_SERVICE
        case respConditionalChangeNAD:
            pDiagTransfer->response.respSId = RSID(SID_ConditionaChangeNad);
            pDiagTransfer->response.dataLen = 0u; /* Only data bytes length */
            break;
#endif /* SL_EN_COND_CHANGE_NAD_SERVICE */

#if SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE)
        case respPositiveAssignFID:
            pDiagTransfer->response.respSId = RSID(SID_AssignFrameId);
            pDiagTransfer->response.dataLen = 0u; /* Only data bytes length */
            break;
#endif /* SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE) */

#if SL_vSAE_J2602_2012
        case respPositiveTargetedReset:
            pDiagTransfer->response.respSId = RSID(SID_J2602_TargetedReset);
            pDiagTransfer->response.dataLen = 5u; /* Only data bytes length */
            pDiagTransfer->response.data[0] = ml_ProductID[0];  /* SupplierID LSB */
            pDiagTransfer->response.data[1] = ml_ProductID[1];  /* SupplierID MSB */
            pDiagTransfer->response.data[2] = ml_ProductID[2];  /* FunctionID LSB */
            pDiagTransfer->response.data[3] = ml_ProductID[3];  /* FunctionID MSB */
            pDiagTransfer->response.data[4] = ml_ProductID[4];  /* VariantID */
            break;

        case respNegativeTargetedReset:
            pDiagTransfer->response.respSId = RSID_NegativeResponse;
            pDiagTransfer->response.dataLen = 5u; /* Only data bytes length */
            pDiagTransfer->response.data[0] = ml_ProductID[0];  /* SupplierID LSB */
            pDiagTransfer->response.data[1] = ml_ProductID[1];  /* SupplierID MSB */
            pDiagTransfer->response.data[2] = ml_ProductID[2];  /* FunctionID LSB */
            pDiagTransfer->response.data[3] = ml_ProductID[3];  /* FunctionID MSB */
            pDiagTransfer->response.data[4] = ml_ProductID[4];  /* VariantID */
            break;
#endif /* SL_vSAE_J2602_2012 */

#if SL_vLIN_2_1_plus || SL_vISO17987_2016
        case respPositiveAssignFidRange:
            pDiagTransfer->response.respSId = RSID(SID_AssignFrameIdRange);
            pDiagTransfer->response.dataLen = 0u; /* Only data bytes length */
            break;
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

#if (SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE
        case respPositiveSaveConfig:
            pDiagTransfer->response.respSId = RSID(SID_SaveConfiguration);
            pDiagTransfer->response.dataLen = 0u; /* Only data bytes length */
            break;
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

#if SL_EN_READ_BY_ID_CALLOUT
        case respUserDefIdCallout:
            /* The buffer is already filled in */
            break;
#endif /* SL_EN_READ_BY_ID_CALLOUT */

        default:
            retVal = false;
            break;
    }   /* switch (thisPendingSlaveResponse) */
    return (retVal);
}


/** Clears the pending slave's response (if any) to be sent with a subsequent Slave response frame.
 * This function should be called by an application to inform the std LIN API to abort
 * a pending network management, transport layer or diagnostics communication.
 * For example in case of reception of a corrupted master request frame.
 * @note The function call shall not interrupt the LIN Slave ISR.
 * @return  void
 */
void ld_DiagClearPendingResponse (void)
{
    pendingSlaveResponse = respNoResponse;
}


/** Returns configured NAD.
 * @param[in] NAD   Checked NAD value.
 * @return Current NAD.
 */
l_u8 ld_GetLINNAD (l_u8 NAD)
{
    (void)NAD; /* There is no interest to track other than current NADs on the bus */
    l_u8 retVal = ml_ConfiguredNAD;
#if SL_EN_ASSIGN_NAD_SERVICE
    /* Assign NAD request */
    if (ML_SLAVE_FRAME_DATA_BUFFER[2] == 0xB0u) {
        retVal = ml_InitialNAD;
    }
#endif /* SL_EN_ASSIGN_NAD_SERVICE */
    return retVal;
}


#if SL_vLIN_2_x || SL_vISO17987_2016 || (SL_vSAE_J2602_2012 && (SL_EN_ASSIGN_NAD_SERVICE || \
                                                                SL_EN_ASSIGN_FRAME_ID_SERVICE || \
                                                                SL_EN_READ_BY_ID_SERVICE))
/** Checks if supplierID is valid, i.e. match node's SupplierID which is located in ml_ProductID[0], ml_ProductID[1].
 *
 * Implementation note:
 * Little-endian CPU is implied and word aligned ml_ProductID[] is required
 *
 * @return Status:
 * - ML_SUCCESS     - supplierID is valid;
 * - ML_FAILURE     - supplierID is not valid.
 */
ml_Status_t isSupplierIdValid (const l_u8 data[])
{
    l_u16 supplierID;
    l_u16 cur_supplierID;
    ml_Status_t status;

    /* supplierID word could be on odd address, so get LSB, MSB separately */
    supplierID = ((l_u16)data[1] << 8 ) | data[0];
    cur_supplierID = (l_u16)ml_ProductID[0]
                     | (((l_u16)ml_ProductID[1]) << 8);

    if (   (supplierID == cur_supplierID)
           || (supplierID == 0x7FFFU /* wildcard */)) {
        status = ML_SUCCESS;
    } else {
        status = ML_FAILURE;
    }

    return status;
}


/** Checks if functionID is valid, i.e. match node's FunctionID which is located in ml_ProductID[2], ml_ProductID[3].
 *
 * Implementation note:
 * Little-endian CPU is implied and word aligned ml_ProductID[] is required
 *
 * @return Status:
 * - ML_SUCCESS  FunctionID is valid;
 * - ML_FAILURE  FunctionID is not valid.
 */
ml_Status_t isFunctionIdValid (const l_u8 data[])
{
    l_u16 functionID;
    l_u16 cur_functionID;
    ml_Status_t status;

    /* functionID word could be on odd address, so get LSB, MSB separately */
    functionID = ((l_u16)data[1] << 8) | data[0];
    cur_functionID = ((l_u16)ml_ProductID[2])
                     | (((l_u16)ml_ProductID[3]) << 8);

    if (   (functionID == cur_functionID)
           || (functionID == 0xFFFFU /* wildcard */)) {
        status = ML_SUCCESS;
    } else {
        status = ML_FAILURE;
    }

    return status;
}
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || (SL_vSAE_J2602_2012 && (SL_EN_ASSIGN_NAD_SERVICE || SL_EN_ASSIGN_FRAME_ID_SERVICE || SL_EN_READ_BY_ID_SERVICE)) */


#if SL_EN_ASSIGN_NAD_SERVICE
/** Parses Assign NAD frame (service 0xB0) and returns resulting slave response.
 * Uses global pDiagTransfer structure pointer.
 *
 *  Frame structure:
 *  InitialNAD, PCI, SID, SupplierID_LSB, SupplierID_MSB, FunctionID_LSB, FunctionID_MSB, NewNAD
 *     [0]      [1]  [2]       [3]             [4]             [5]            [6]           [7]
 *
 * @return: Requested response or respNoResponse
 */
static SlaveResponse_t assignNAD (void)
{
    if (pDiagTransfer->request.dataLen != 0x05u) {
        /* if (PCI-1) is not valid .. */
        return respNoResponse;
    }
    /* else: (PCI-1) is valid, continue processing .. */

    /* Validate SupplierID [3..4] and FunctionID [5..6] */
    if ( (isSupplierIdValid(&pDiagTransfer->request.data[0]) == ML_SUCCESS) &&
         (isFunctionIdValid(&pDiagTransfer->request.data[2]) == ML_SUCCESS) ) {
        ml_ConfiguredNAD = pDiagTransfer->request.data[4];   /* new NAD */
        return respPositiveAssignNAD;
    } else {
        return respNoResponse;
    }
}
#endif /* SL_EN_ASSIGN_NAD_SERVICE */


#if SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE)
/** Parses Assign Frame Id frame (service 0xB1), performs requested assignment and returns resulting slave response.
 *
 * Uses global pDiagTransfer structure pointer.
 *
 *  Frame structure:
 *  NAD, PCI, SID, SupplierID_LSB, SupplierID_MSB, MessageID_LSB, MessageID_MSB, PID
 *  [0]  [1]  [2]       [3]             [4]             [5]           [6]        [7]
 *
 * @return:
 * - respPositiveAssign   - Frame ID successfully assigned to the message;
 * - respNoResponse       - assignment failed.
 */
static SlaveResponse_t assignFrameID (void)
{
    l_u16 msgID_lin20; /* LIN 2.0 message identifier (16-bit) */
    l_u8 pid;
    l_u8 index;


    if (pDiagTransfer->request.dataLen != 0x05u) {
        /* if (PCI-1) is not valid .. */
        return respNoResponse;
    }
    /* else: (PCI-1) is valid, continue processing .. */

    if ( isSupplierIdValid(&pDiagTransfer->request.data[0]) == ML_SUCCESS ) {
        /* if supplier ID is valid .. */
        msgID_lin20 = ((l_u16)pDiagTransfer->request.data[3] << 8) | pDiagTransfer->request.data[2];
        pid = pDiagTransfer->request.data[4];

        /* Find the internal index of requested message */
        for (index = 0U; index < SL_NUMBER_OF_DYNAMIC_MESSAGES; index++) {
            if (MID_list[index] == msgID_lin20) {
                break; /* found an index */
            }
        }

        if (index < SL_NUMBER_OF_DYNAMIC_MESSAGES) {
            ml_Status_t status;

            /*
             * 0x40 (non valid Protected ID) is used to unassign FrameID for this message
             * [see LIN 2.0 spec, page 121]. Unassigned FrameID can be used for other
             * message then.
             *
             * MLX4 LIN FW does not remove message-to-FrameID association from internal
             * RAM (LIN_IDs array), but marks the message as disabled. So this message
             * is excluded from FrameId-to-message_index mapping
             */
            if (0x40U == pid) {
                (void)ml_Disconnect();
                status = ml_DisableMessage(index);
                (void)ml_Connect();

#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
                SL_EN_SAVE_CONFIG_FUNC)
                node_configuration[index] = 0x00U;
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */
            } else if ((pid & 0x3FU) < 0x3CU) {
                (void)ml_Disconnect();
                status = ml_AssignFrameToMessageID(index, pid);
                (void)ml_Connect();

#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
                SL_EN_SAVE_CONFIG_FUNC)
                node_configuration[index] = pid;    /* save assignment to node_configuration */
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */
            }
            /* Frame Ids 0x3C..0x3F shall not be re-assigned [LIN spec] */
            else {
                status = ML_FAILURE;
            }

            if (ML_SUCCESS == status) {
#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
                /*
                 * For Associated Unconditional Frames update PID
                 */
                if (sft_UncondAssociatedFrame == frameList[index].FrameType) {
                    l_s_AssociatedUncondFrame_t *assocFrame = (l_s_AssociatedUncondFrame_t*)frameList[index].Frame;
                    assocFrame->PID = pid;
                }
                /* else: don't store PID */
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */
                return respPositiveAssignFID;
            }
            /* else : Assignment failed */
        }
        /* else : index is not found */
    }
    /* else: wrong SupplierID  */

    return respNoResponse;
}
#endif /* SL_vLIN_2_0 || (SL_vSAE_J2602_2012 && SL_EN_ASSIGN_FRAME_ID_SERVICE) */


#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE
/** Parses Read by Identifier frame (service 0xB2) and returns resulting slave response.
 *
 * Uses global pDiagTransfer structure pointer.
 *
 *  Frame structure:
 *  NAD, PCI, SID, Identifier, SupplierID_LSB, SupplierID_MSB, FunctionID_LSB, FunctionID_MSB
 *  [0]  [1]  [2]     [3]            [4]             [5]           [6]              [7]
 *
 * @return: Requested response or respNoResponse.
 */
static SlaveResponse_t readByIdentifier (void)
{
    SlaveResponse_t retVal;

    if (pDiagTransfer->request.dataLen != 0x05u) {
        /* if (PCI-1) is not valid .. */
        retVal = respNoResponse;
    } else {
        /* else: (PCI-1) is valid, continue processing .. */

        /* Validate SupplierID [4..5] and FunctionID [6..7] */
        if ( (isSupplierIdValid(&pDiagTransfer->request.data[1]) == ML_SUCCESS) &&
             (isFunctionIdValid(&pDiagTransfer->request.data[3]) == ML_SUCCESS) ) {

            /* if supplier/function IDs are valid .. */

            switch (pDiagTransfer->request.data[0]) {    /* check IDs ..  */
                case 0x00:                      /* ID: LIN Product Identification */
                    retVal = respLinProductID;
                    break;

#if SL_EN_SERIAL_NUMBER_CALLOUT
                case 0x01:                      /* ID: Serial number (optional) */
                    retVal = respSerialNumber;
                    break;
#endif /* SL_EN_SERIAL_NUMBER_CALLOUT */

#if SL_vISO17987_2016 && SL_EN_READ_BY_ID_BIT_TIMING_TEST
                case 0x02:                      /* ID: bit timing test (optional) */
                    retVal = respNegativeResponse;
                    break;
#endif /* SL_vISO17987_2016 && SL_EN_READ_BY_ID_BIT_TIMING_TEST */

#if SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION
                case 0x03:                      /* ID: NCF/LDF version (optional) */
                    retVal = respNcfLdfVersion;
                    break;
#endif /* SL_vISO17987_2016 && SL_EN_NCF_LDF_VERSION */

                default:
#if SL_EN_READ_BY_ID_CALLOUT
                    if ( (pDiagTransfer->request.data[0] > 31u ) && (pDiagTransfer->request.data[0] < 64u) ) {
                        l_u8 pci = 6; /* Default value */
                        l_u8 response = ld_read_by_id_callout (dummy,
                                                               pDiagTransfer->request.data[0],
                                                               (l_u8*)&pci,
                                                               pDiagTransfer->response.data);
                        pDiagTransfer->response.dataLen = (l_u16)pci - 1u; /* Convert PCI to data length */
                        pDiagTransfer->response.respSId = RSID(SID_ReadById);
                        retVal = (SlaveResponse_t)response;
                        break;
                    }
                    /* else : wrong ID */
#endif /* SL_EN_READ_BY_ID_CALLOUT */
                    retVal = respNegativeResponse;
                    break;
            }
        } else {
            /* SupplierID or FunctionID are not matched */
            retVal = respNoResponse;
        }
    }

    return retVal;
}
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_EN_READ_BY_ID_SERVICE */


#if SL_EN_COND_CHANGE_NAD_SERVICE
/** Parses Conditional change NAD frame (service 0xB3) and returns resulting slave response.
 *
 * Uses global pDiagTransfer structure pointer.
 *
 *  Frame structure:
 *  NAD, PCI, SID, Identifier, Byte, Mask, Invert, New NAD
 *  [0]  [1]  [2]     [3]       [4]   [5]    [6]     [7]
 *
 * @return: Requested response or respNoResponse
 */
static SlaveResponse_t conditionalChangeNAD (void)
{
    if (pDiagTransfer->request.dataLen == 0x05u) {
        /* if PCI-1 is valid .. */
        l_u8 data_bytes[4];

        /* Get the identifier of possible read by ID response and selected by Id
         * Extract the data byte selected by `Byte` (starts from 1)
         */
        switch (pDiagTransfer->request.data[0]) { /* Requested Id */
            case 0x00: /* ID: LIN Product Identification */
                if (   (pDiagTransfer->request.data[1] == 0u)
                       || (pDiagTransfer->request.data[1] > 5u)) {
                    /* if selected Byte is out of range [1..5] .. */
                    return respNoResponse; /* .. no response */
                }
                /* else: correct Byte selected, continue processing .. */

                data_bytes[0] = ml_ProductID[pDiagTransfer->request.data[1] - 1u]; /* DataByte_1 is ml_ProductID[0] */
                break;

#if SL_EN_SERIAL_NUMBER_CALLOUT
            case 0x01: /* ID: Serial number (optional) */
                if (   (pDiagTransfer->request.data[1] == 0u)
                       || (pDiagTransfer->request.data[1] > 4u)) {
                    /* if selected Byte is out of range [1..4] .. */
                    return respNoResponse; /* .. no response */
                }
                /* else: correct Byte selected, continue processing .. */

                ld_serial_number_callout(data_bytes); /* the data_bytes buffer is filled in here */
                data_bytes[0] = data_bytes[pDiagTransfer->request.data[1] - 1u];
                break;
#endif /* SL_EN_SERIAL_NUMBER_CALLOUT */

            default:    /* Other IDs */
#if SL_EN_READ_BY_ID_CALLOUT
                if ( (pDiagTransfer->request.data[0] > 31u ) && (pDiagTransfer->request.data[0] < 64u) ) {
                    /* if ID is correct .. */
                    l_u8 status;

                    if ((pDiagTransfer->request.data[1] == 0u)
                        || (pDiagTransfer->request.data[1] > 5u)) { /* if selected Byte is out of range [1..5] .. */
                        return respNoResponse;  /* no response */
                    }

                    /* else: correct Byte selected, continue processing .. */

                    /* Only function return value is used */
                    l_u8 pci;
                    l_u8 rbi_data[5];
                    status = ld_read_by_id_callout (dummy, pDiagTransfer->request.data[0], &pci, rbi_data);

                    if (status != LD_POSITIVE_RESPONSE) {
                        /* if not a positive response .. */
                        return respNoResponse;
                    }
                    /* else: positive response expected */

                    data_bytes[0] = rbi_data[pDiagTransfer->request.data[1] - 1u]; /* Byte_1 corresponds to data[0] */
                } else {   /* Requested Id is not supported .. */
                    return respNoResponse;
                }
#else /* SL_EN_READ_BY_ID_CALLOUT */
                return respNoResponse;
#endif /* SL_EN_READ_BY_ID_CALLOUT */
                break;      /*lint !e527: unreachable because or the return above */
        }

        data_bytes[0] ^= pDiagTransfer->request.data[3]; /* do a bitwise XOR with Invert */
        data_bytes[0] &= pDiagTransfer->request.data[2]; /* do a bitwise AND with Mask */

        if (data_bytes[0] == 0U) {                      /* if condition PASSED .. */
            ml_ConfiguredNAD = pDiagTransfer->request.data[4]; /* new NAD */
            return respConditionalChangeNAD;
        }
        /* else : condition FAILED => no response  */
    }
    /* else : PCI is not correct => no response */
    return respNoResponse;
}
#endif /* SL_EN_COND_CHANGE_NAD_SERVICE */


#if (SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE
/** Process Save Configuration frame.
 * @return  void
 */
static SlaveResponse_t saveConfiguration (void)
{
    if (pDiagTransfer->request.dataLen != 0u) {
        /* if (PCI-1) is not correct .. */
        return respNoResponse;
    }
    /* else: (PCI-1) is correct, continue processing .. */

    s_ifcStatus.mapped.SaveConfig = true;

    return respPositiveSaveConfig;
}
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */


#if SL_vLIN_2_1_plus || SL_vISO17987_2016
/** Process Assign Frame Id Range frame.
 * The function corresponds to 0xB7 service ID, is used by ld_DiagRequest
 *
 * Various pid values:
 * 0xFF is used as do-not-care value, used to keep other assigned values of this frame.
 * 0x00 (non valid Protected ID) is used in this request to unassign FrameId
 * for this message [see LIN 2.1 spec, page 74]. Unassigned FrameId can be used
 * for other message then.
 *
 * MLX4 LIN FW does not remove message-to-FrameId association from internal
 * RAM (LIN_IDs array), but marks the message as disabled.
 *
 * Uses global pDiagTransfer structure pointer.
 *
 *  Frame structure:
 *  NAD, PCI, SID, start_idx, PID(idx), PID(idx+1), PID(idx+2), PID(idx+3)
 *  [0]  [1]  [2]     [3]        [4]       [5]         [6]         [7]
 *
 * @return: Requested response or respNoResponse.
 */
static SlaveResponse_t assignFrameIdRange (void)
{
    /* wrong PCI */
    if (pDiagTransfer->request.dataLen != 0x05U) { /* if (PCI-1) is not correct .. */
        return respNoResponse;
    }

    /* invalid start index */
    if (pDiagTransfer->request.data[0] >= SL_NUMBER_OF_DYNAMIC_MESSAGES) {
        return respNoResponse;
    } else {
        l_u8 *pid;
        l_u8 index;
        l_u8 i;

        index = pDiagTransfer->request.data[0] + 3u; /* index of the last frame in the range */
        pid = &pDiagTransfer->request.data[4]; /* PID of the last frame in the range */

        for (i = 0U; i < 4U; i++) {
            if (*pid != 0xFFU) {                                /* if id is NOT 0xFF .. */
                if (index >= SL_NUMBER_OF_DYNAMIC_MESSAGES) {   /* if wrong index for valid pid .. */
                    return respNoResponse;
                } else {                                          /* proper index */
                    (void)ml_Disconnect();
                    if (*pid == 0x00U) {                        /* unassign */
                        if (ml_DisableMessage(index) != ML_SUCCESS) {
                            (void)ml_Connect();
                            return respNoResponse;              /* failed to unassign */
                        }
                        /* else save unassignment to node_configuration */

#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
                        SL_EN_SAVE_CONFIG_FUNC)
                        node_configuration[index] = 0x00U;
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */
                    } else {                                      /* assign */
                        if (ml_AssignFrameToMessageID(index,*pid) != ML_SUCCESS) {
                            (void)ml_Connect();
                            return respNoResponse; /* failed to assign */
                        }
                        /* else: assignment was OK, continue processing */

#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
                        SL_EN_SAVE_CONFIG_FUNC)
                        /* save assignment to node_configuration */
                        node_configuration[index] = *pid;
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */

#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
                        /*
                         * For Associated Unconditional Frames update PID
                         */
                        if (sft_UncondAssociatedFrame == frameList[index].FrameType) {
                            l_s_AssociatedUncondFrame_t *assocFrame =
                                (l_s_AssociatedUncondFrame_t*)frameList[index].Frame;
                            assocFrame->PID = *pid;
                        }
                        /* else: don't store PID */
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */
                    } /* assign */

                    (void)ml_Connect();
                } /* proper index */
            } else {   /* id is 0xFF .. */
                       /* .. skip PID from processing */
            }

            --pid;          /*lint !e960 : deviation MISRA 2004:17.4 pointer decrement */
            --index;        /* get previous index */
        } /* for 4 pids */

        return respPositiveAssignFidRange;
    }
} /* assignFrameIdRange */
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */

#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

#endif /* LIN_SLAVE_API == 1 */
/* EOF */
