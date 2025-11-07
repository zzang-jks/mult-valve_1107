/**
 * @file lin_ldr.c
 * @brief Central module of the EOL FastLIN layer. Token: ldr
 * @internal
 *
 * @copyright (C) 2016-2017 Melexis N.V.
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
 * @ingroup mlx_lin_ldr
 *
 * @details
 */

#include "lin_ldr.h"

#include "mlx_data_transfer.h"
#include "read_write_flash.h"
#include "read_write_eeprom.h"
#include "read_write_ram.h"
#include "protocol_extension.h"


/*-----------------------------------------------------------------------------
 * Constants and macros                 (scope: module-local)
 */

#define ML_ERROR_BIT    (1u << 7)   /**< mask for error bit pos */

#define LIN_ADDRESS_BLK1_IX (1)         /**< offset in message address */
#define LIN_ADDRESS_BLK0_IX (2)         /**< offset in message address */
#define LIN_ADDRESS_REM1_IX (3)         /**< offset in message address */
#define LIN_ADDRESS_REM0_IX (4)         /**< offset in message address */
#define BITS_IN_BYTE        (8)

#define LDR_MELEXIS_LOADER_MIN 1u


/*-----------------------------------------------------------------------------
 * Declaration of variables             (scope: module-local)
 */

LDR_t* ldrMe; //TODO: Add the error handling


/*-----------------------------------------------------------------------------
 * Function prototypes                  (scope: module-local)
 */

extern bool ldr_Request(LINDiagTransfer_t *transfer);
extern bool ldr_ResponseTransmitted(LINDiagTransfer_t *transfer);

static void ldr_HandleMLXLoaderRequests(LINDiagTransfer_t* transfer);
#if LDR_HAS_PARITY_CHECK && LDT_HAS_FLASH_FUNCTIONS
static bool ldr_isValideCmd(uint8_t cmd);
#endif

#if !defined(LDT_DISABLE_RAM_EXEC_FUNCTIONS) || (LDT_DISABLE_RAM_EXEC_FUNCTIONS == 0)
static void ldr_ExecuteRam(LINDiagTransfer_t *transfer);
#endif


/*-----------------------------------------------------------------------------
 * Function implementation
 */

/** Initializes LIN Loader RAM table.
 * Initializes LIN Loader RAM pointer with reserved space.
 *
 * Note: It must be executed at the very beginning of LIN Loader initialization procedure.
 *
 * @param[in] resRamTable Pointer to reserved space for LIN Loader RAM table
 * @return void
 */
void ldr_InitRamTable (ldr_RamFunctions_t *resRamTable)
{
    /* Init global pointer with reserved memory */
    pldrRamFunctions = resRamTable;
}

void ldr_Init(LDR_t * privateVariables)
{
    ldrMe = privateVariables;
    (void)ldt_SubscribeMultipleHandler(plinEventTable->ldt_Request, ldr_Request);
    (void)ldt_SubscribeMultipleHandler(plinEventTable->ldt_ResponseTransmitted, ldr_ResponseTransmitted);

    ldrMe->memoryAddress = LDR_PROG_FLASH_START_INDEX;
    ldrMe->memorySize = LDR_FLASH_PAGE_BYTE_SIZE;
    ldrMe->selectedBlock = LB_FLASH;
    ldrMe->protExt.peCode = peReadFlashModify;
    ldrMe->protExt.peValue = rfmNormal; /* Don't modify Read Flash command behaviour */

    LDR_DEBUG_OUTPUT("LDR: LIN Loader init is done");
}

bool ldr_ResponseTransmitted(LINDiagTransfer_t *transfer)
{
    (void) transfer;

    /* NOTE: by default the frames aren't used and available for the handler on the level above */
    bool retVal = false;

    if (ldrMe->commandID == CMD_FAST_PROTOCOL) {
        (void)ml_Disconnect();
        ldtMe->ml_ProtocolMode = ML_PM_FASTLIN;
        ml_InitFast(ldrMe->protExt.peValue, LDR_MLX4_FPLL);

        /* Loader expects to receive SID=B4 (Data Dump) for the Fast LIN frames */
        (void)ml_ReleaseBufferProg(ML_TRUE);

        retVal = true;
        LDR_DEBUG_OUTPUT("LDR: Fast mode is enabled");
    }

    return retVal;
}

bool ldr_Request(LINDiagTransfer_t *transfer)
{
    bool retVal = false;

    /* To check if the message is used or not.
     * The LIN Loader operations which don't prepare the answer
     * but react on the request message shall change the reqSID or respSID. */
    uint8_t initialSID = transfer->request.reqSId;

    if (transfer->request.reqSId == LDR_REQ_DATA_DUMP) {
        if (ldrMe->MlxLinLdrSession == true) {
            if (transfer->request.dataLen >= LDR_MELEXIS_LOADER_MIN) {
                ldr_HandleMLXLoaderRequests(transfer);
            }
#if LDR_HAS_NEGATIVE_RESPONSES
            else {
                ldr_BuildNegativeResponse(transfer, MLX16ERROR_INCORRECT_OPERATION);
            }
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
        } else {
            /* ignore request when MlxLinLdrSession is disabled */
            LDR_DEBUG_OUTPUT("LDR: Ignore request when MlxLinLdrSession is disabled");
        }
    } else {
        /* ignore non Data Dump SIDs */
        LDR_DEBUG_OUTPUT("LDR: Ignore request with non Data Dump SIDs");
    }

    if (transfer->response.respSId != initialSID) {
        /* the frame has been used */
        retVal = true;
    } else {
        /* keep the retVal == false */

        LDR_DEBUG_OUTPUT("LDR: Loader has no response");
    }

    return retVal;
}

bool ldr_HandleEnterProgMode(LINDiagTransfer_t* transfer)
{
    bool retVal = false;

    LDR_DEBUG_OUTPUT("LDR: Enter programming mode is requested");

    if (ldrMe->MlxLinLdrSession == false) {
        LDR_DEBUG_OUTPUT("LDR: handler ldr_ApplicationStop");
        if (pldrRamFunctions->ldr_ApplicationStop() == true) {
            LDR_DEBUG_OUTPUT("LDR: handler ldr_EnterProgMode");
            pldrRamFunctions->ldr_EnterProgMode();
            /* For devices executing a chip reset to enter programming mode
             * ldr_EnterProgMode() will not return and the positive response
             * is sent after reset */
            pldrRamFunctions->ldr_BuildPositiveResponseEnterProg(transfer);
            retVal = true;

            LDR_DEBUG_OUTPUT("LDR: Programming mode is entered");
        }
    } else {
        /* Always answer with positive response when MlxLinLdrSession is already enabled */
        LDR_DEBUG_OUTPUT("LDR: handler ldr_BuildPositiveResponseEnterProg");
        pldrRamFunctions->ldr_BuildPositiveResponseEnterProg(transfer);
        retVal = true;

        LDR_DEBUG_OUTPUT("LDR: The programming mode is already entered");
    }
    return retVal;
}

static void ldr_HandleMLXLoaderRequests(LINDiagTransfer_t* transfer)
{
    uint8_t cmdID = transfer->request.data[LDR_DATAI_MLXCOMMAND];
    bool ldrIsValid = true;

#if LDT_HAS_FLASH_FUNCTIONS
    if (((cmdID & 0x30u) == CMD_READ_FLASH) || ((cmdID & 0x30u) == CMD_WRITE_FLASH)) {
        /* Decode CMD_READ_FLASH and CMD_WRITE_FLASH
         * 4 lowbits of the commandID are address bits of 20Bit address */
#if LDR_HAS_PARITY_CHECK
        ldrIsValid = ldr_isValideCmd(cmdID);
        if (ldrIsValid == false) {
  #if LDR_HAS_NEGATIVE_RESPONSES
            ldr_BuildNegativeResponse(transfer, MLX16ERROR_INCORRECT_OPERATION);
  #endif
        }
        else
        {
            cmdID &= 0x30u;
        }
#endif
    }
#endif

    if (ldrIsValid == true) {
#if LDT_HAS_GET_STATUS_WRITE_CHECK
        uint8_t previousCmdID = ldrMe->commandID;
#endif
        ldrMe->commandID = cmdID;

        switch (cmdID) {

            case CMD_FAST_PROTOCOL:
            {
                LDR_DEBUG_OUTPUT("LDR: Fast mode is requested");
                ldrMe->protExt.peValue = transfer->request.data[1];

                if (ldrMe->protExt.peValue > LDR_MAX_FLEX_FAST_BAUDRATE_K) {
                    ldrMe->protExt.peValue = LDR_MAX_FLEX_FAST_BAUDRATE_K;
                } else if (ldrMe->protExt.peValue < LDR_MIN_FAST_BAUDRATE_K) {
                    ldrMe->protExt.peValue = LDR_MIN_FAST_BAUDRATE_K;
                } else {
                    /* Leave initial value */
                }

                /* Prepare response */
                LINDiagResponse_t *response = &(transfer->response);
                response->respSId = LDR_RES_DATA_DUMP;
                response->data[0] = 0;
                response->data[1] = ldrMe->protExt.peValue;
                break;
            }
            case CMD_RESTART:
                LDR_DEBUG_OUTPUT("LDR: The MLX16 reset is requested");
                ldr_ResetMlx16();
                break;
            case CMD_STATUS:
#if LDT_HAS_GET_STATUS_WRITE_CHECK
                if ((previousCmdID != CMD_WRITE_FLASH) ||
                    ((ldrMe->memorySize % LDR_FLASH_PAGE_BYTE_SIZE) == 0)) {
#endif
                    LDR_DEBUG_OUTPUT("LDR: The status is requested");
                    ldr_BuildPositiveResponseMlxCmd(transfer);
#if LDT_HAS_GET_STATUS_WRITE_CHECK
                } else
                {
                    ldrMe->commandID = previousCmdID;// CMD_WRITE_FLASH
                    LDR_DEBUG_OUTPUT("LDR: Write size not ok");
                    ldr_BuildNegativeResponse(transfer, MLX16ERROR_TOO_MANY_DATA_FRAME_RECEIVED);
                }
#endif
                break;

#if LDT_HAS_FLASH_FUNCTIONS || LDT_HAS_NVRAM_FUNCTIONS || LDT_HAS_EEPROM_FUNCTIONS || LDT_HAS_RAM_FUNCTIONS
            case CMD_DATA:
                LDR_DEBUG_OUTPUT("LDR: The data dump frame is requested");
                data_Handle(transfer);
                break;
#endif

#if LDT_HAS_TABLE_FUNCTIONS
            case CMD_READ_TABLE:
                LDR_DEBUG_OUTPUT("LDR: Read table frame is requested");

                /* Stub return positive result: 0x2 0x1*/
                ldr_BuildPositiveResponseMlxCmd(transfer);
                transfer->response.data[1] = 1; /* Status byte */
                transfer->response.dataLen++;
                break;
#endif

#if LDT_HAS_NVRAM_FUNCTIONS == 1
            case CMD_RESTORE_NVRAM:
                LDR_DEBUG_OUTPUT("LDR: Restore NVRAM frame is requested");
                ldr_NvramLoadAll();
                ldr_BuildPositiveResponseMlxCmd(transfer);
                break;
            case CMD_SAVE_NVRAM:
                LDR_DEBUG_OUTPUT("LDR: Save NVRAM frame is requested");
                ldr_NvramSaveAll();
                ldr_BuildPositiveResponseMlxCmd(transfer);
                break;
#endif /* LDT_HAS_NVRAM_FUNCTIONS */

#if LDT_HAS_EEPROM_FUNCTIONS == 1
            case CMD_READ_EEPROM:
                LDR_DEBUG_OUTPUT("LDR: Read EEPROM frame is requested");
                rwe_Handle(transfer);
                break;

            case CMD_WRITE_EEPROM:
                LDR_DEBUG_OUTPUT("LDR: Write EEPROM frame is requested");
                rwe_Handle(transfer);
                break;
#endif /* LDT_HAS_EEPROM_FUNCTIONS */

#if LDR_HAS_PROTOCOL_EXTENSION
            case CMD_PROT_EXTENSION:
                LDR_DEBUG_OUTPUT("LDR: Protocol extension frame is requested");
                pe_Handle(transfer);
                break;
#endif /* LDR_HAS_PROTOCOL_EXTENSION */

#if LDT_HAS_RAM_FUNCTIONS
            case CMD_WRITE_RAM:
                LDR_DEBUG_OUTPUT("LDR: Write RAM frame is requested");
                rwr_Handle(transfer);
                break;
            case CMD_READ_RAM:
                LDR_DEBUG_OUTPUT("LDR: Read RAM frame is requested");
                rwr_Handle(transfer);
                break;
#endif

#if !defined(LDT_DISABLE_RAM_EXEC_FUNCTIONS) || (LDT_DISABLE_RAM_EXEC_FUNCTIONS == 0)
            case CMD_EXECUTE_RAM:
                LDR_DEBUG_OUTPUT("LDR: Execute RAM frame is requested");
                ldr_ExecuteRam(transfer);
                break;
#endif

#if LDT_HAS_FLASH_FUNCTIONS
            case CMD_READ_FLASH:
            case CMD_WRITE_FLASH:
            case CMD_ERASE_FLASH:
                LDR_DEBUG_OUTPUT("LDR: Flash read/write/erase frame is requested");
                rwf_Handle(transfer);
                break;
#endif /* LDT_HAS_FLASH_FUNCTIONS */

            default:
#if LDR_HAS_NEGATIVE_RESPONSES
                LDR_DEBUG_OUTPUT("LDR: Unsupported operation");
                ldr_BuildNegativeResponse(transfer, MLX16ERROR_INCORRECT_OPERATION);
#endif
                break;
        }
    }
}

#if !defined(LDT_DISABLE_RAM_EXEC_FUNCTIONS) || (LDT_DISABLE_RAM_EXEC_FUNCTIONS == 0)
static void ldr_ExecuteRam(LINDiagTransfer_t *transfer)
{
    ldr_BuildPositiveResponseMlxCmd(transfer);
    ldr_ExecuteRam_t execute_ram = (ldr_ExecuteRam_t)(((uint16_t)transfer->request.data[LIN_ADDRESS_BLK1_IX] << BITS_IN_BYTE)
                                                      | (uint16_t)transfer->request.data[LIN_ADDRESS_BLK0_IX]);
    execute_ram(); /* Call the RAM */
}
#endif

#if LDR_HAS_PARITY_CHECK && LDT_HAS_FLASH_FUNCTIONS
/* Validates the two parity bits (bit6 and 7) in the command ID
 * Currently only used for CMD_READ_FLASH and
 * CMD_WRITE_FLASH where 4 low bits are extended address bits of 20bit address.
 * All other commands have fixed parity bits.
 * return != 0 valid Command ID
 *        == 0 invalid command ID
 *
 */
static bool ldr_isValideCmd(uint8_t cmd)
{
    uint8_t b6 = 1 & (cmd >> 6);
    uint8_t b7 = 1 & (cmd >> 7);

    uint8_t p0 = 1 & ((cmd >> 0) ^ (cmd >> 1) ^ (cmd >> 2) ^ (cmd >> 4));
    uint8_t p1 = 1 & ((cmd >> 1) ^ (cmd >> 3) ^ (cmd >> 4) ^ (cmd >> 5) ^ 1);

    return (!((p1 ^ b7) || (p0 ^ b6)));
}
#endif /* LDR_HAS_NEGATIVE_RESPONSES */

#if LDR_HAS_NEGATIVE_RESPONSES
void ldr_BuildNegativeResponse(LINDiagTransfer_t *transfer, LdrMlx16Error_t mlx16Error)
{
    transfer->response.respSId = 0x7F; /* TODO: Replace with a constant */
    transfer->response.data[0] = ML_ERROR_BIT | LDR_STATE;
    //TODO: MLX4 errors can be reported as well. Error event handler can be used when EOL infinite loop is enabled
    transfer->response.data[1] = mlx16Error << 4; /* MLX4 errors are not reported */
    transfer->response.dataLen = 2;

    LDR_DEBUG_OUTPUT("LDR: Negative response has been prepared");
}
#endif /* LDR_HAS_NEGATIVE_RESPONSES */

void ldr_BuildPositiveResponseMlxCmd(LINDiagTransfer_t* transfer)
{
    LINDiagResponse_t *response = &(transfer->response);
    response->respSId = LDR_RES_DATA_DUMP;
    response->data[0] = LDR_STATE;
    response->dataLen = 1;

    LDR_DEBUG_OUTPUT("LDR: Status positive response has been prepared");
}

#if LDT_HAS_FLASH_FUNCTIONS || LDT_HAS_NVRAM_FUNCTIONS || LDT_HAS_EEPROM_FUNCTIONS || LDT_HAS_RAM_FUNCTIONS
void ldr_BuildPositiveResponseWriteMem(LINDiagTransfer_t* transfer, bool isWriteTO)
{
    LINDiagResponse_t *response = &(transfer->response);
    response->respSId = LDR_RES_DATA_DUMP;
    response->data[0] = LDR_STATE;
    response->data[1] = (uint8_t) (ldrMe->memorySize >> 8);
    response->data[2] = (uint8_t) (ldrMe->memorySize);
    response->data[3] = 0; /* the response Timeout is constrained by byte value (data[4]) = 255ms */

    if (isWriteTO == true) {
        LDR_DEBUG_OUTPUT("LDR: handler ldr_GetWriteTimeout_ms");
        response->data[4] = pldrRamFunctions->ldr_GetWriteTimeout_ms();
    } else {
        response->data[4] = 0; /* use default values from PC tool */
    }
    response->dataLen = 5;

    LDR_DEBUG_OUTPUT("LDR: Write memory positive response has been prepared");
}

#if !LDR_HAS_STUB_READ_MEM
void ldr_BuildPositiveResponseReadMem(LINDiagTransfer_t* transfer, uint32_t *p_src_add_20bits, uint16_t memorySize)
{
    LINDiagResponse_t *response = &(transfer->response);

    LDR_DEBUG_OUTPUT("LDR: handler ldr_ReadMem");
    if (pldrRamFunctions->ldr_ReadMem(&(response->data[1]), p_src_add_20bits, memorySize) == false) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
#endif
    } else {
        response->respSId = LDR_RES_DATA_DUMP;
        response->data[0] = LDR_STATE; /* Status byte */
        response->dataLen = memorySize + 1;
    }

    LDR_DEBUG_OUTPUT("LDR: Read memory positive response has been prepared");
}
#endif /* !LDR_HAS_STUB_READ_MEM */
#endif

/* EOF */
