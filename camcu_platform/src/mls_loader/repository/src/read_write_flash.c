/**
 * @file read_write_flash.c
 * @brief MlxEOL Read Flash and Write Flash services. Token: rwf
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
#include "protocol_extension.h"


#if LDT_HAS_FLASH_FUNCTIONS == 1


/*-----------------------------------------------------------------------------
 * Constants and macros                 (scope: module-local)
 */

#define LIN_ADDRESS_BLK1_IX (1)         /**< offset in message address */
#define LIN_ADDRESS_BLK0_IX (2)         /**< offset in message address */
#define LIN_ADDRESS_REM1_IX (3)         /**< offset in message address */
#define LIN_ADDRESS_REM0_IX (4)         /**< offset in message address */
#define BITS_IN_BYTE        (8)

#define WF_ADDRESS_BIT_MASK 0xFu

#ifndef LDR_MAX_FLASH_DATA_BYTE_SIZE
#define LDR_MAX_FLASH_DATA_BYTE_SIZE    LDR_FLASH_PAGE_BYTE_SIZE
#endif


/*-----------------------------------------------------------------------------
 * Function prototypes                  (scope: module-local)
 */

#if LDR_HAS_STUB_READ_MEM
static void ldr_BuildPositiveResponseReadMemStubbed(LINDiagTransfer_t* transfer);
#endif /* LDR_HAS_STUB_READ_MEM */


/*-----------------------------------------------------------------------------
 * Function implementation
 */

void rwf_Handle(LINDiagTransfer_t* transfer)
{
    LINDiagRequest_t *request = &(transfer->request);
    bool readFlash = false;
    bool eraseFlash = false;

    /* This is here to support the CMD_ERASE_FLASH and don't take a lot memory */
    if (request->data[0] == CMD_ERASE_FLASH) { /* Erase */
        eraseFlash = true;
    } else {
        uint8_t cmdID = request->data[0] & 0x30;
        if (cmdID == 0x20) { /* Write */
            /* keep the readFlash == false */
        } else if (cmdID == 0x30) { /* Read */
            readFlash = true;
#if LDR_HAS_STUB_READ_MEM
            /* Minimum read flash implementation as required by EOL Flashprocess. */
            ldr_BuildPositiveResponseReadMemStubbed(transfer);
            return;
#endif /* LDR_HAS_STUB_READ_MEM */
        } else {
#if LDR_HAS_NEGATIVE_RESPONSES
            ldr_BuildNegativeResponse(transfer, MLX16ERROR_INCORRECT_OPERATION);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
            return;
        }
    }

    /* Get the 16-bit Flash target address */
    uint32_t memoryAddress_20bits = request->data[LIN_ADDRESS_BLK1_IX];
    memoryAddress_20bits <<= BITS_IN_BYTE;
    memoryAddress_20bits |= request->data[LIN_ADDRESS_BLK0_IX];

    /* Get the 20-bit Flash target address */
    uint32_t memoryAddExt;
    if (eraseFlash == true) { /* Erase */
        /* TODO: Document: High nibble of data len word is used for Flash erase address extension up to 20-bits */
        memoryAddExt = request->data[LIN_ADDRESS_REM1_IX] & (WF_ADDRESS_BIT_MASK << 4);
        memoryAddExt <<= BITS_IN_BYTE + 4;
    } else {
        memoryAddExt = request->data[LDR_DATAI_MLXCOMMAND] & WF_ADDRESS_BIT_MASK;
        memoryAddExt <<= 16;    /* To store 4 high bits in [19:16] of memoryAddExt */
    }
    memoryAddress_20bits |= memoryAddExt;

    /* Get memory size */
    uint16_t memorySize;

    if (eraseFlash == true) { /* Erase */
        /* Only low nibble is used for data len */
        memorySize = request->data[LIN_ADDRESS_REM1_IX] & WF_ADDRESS_BIT_MASK;
    } else {
        memorySize = request->data[LIN_ADDRESS_REM1_IX];
    }
    memorySize <<= BITS_IN_BYTE;
    memorySize |= request->data[LIN_ADDRESS_REM0_IX];

    /* Check if it's CRC calculation request */
#if LDR_HAS_PROTOCOL_EXTENSION
    if ((readFlash == true)
        && (ldrMe->protExt.peCode == peReadFlashModify)
        && (ldrMe->protExt.peValue == rfmCrcCalc)) {
        /* Answer with CRC of the specified memory block */
        pe_BuildPositiveResponseReadCRC(transfer, &memoryAddress_20bits, memorySize);
        return;
    } else {
        /* To continue with action for read/erase/write */
    }
#endif /* LDR_HAS_PROTOCOL_EXTENSION */

    /* Provide the checks after known start address and data length */
#if !LDR_HAS_STUB_READ_MEM
    if ((memoryAddress_20bits < (uint32_t)LDR_PROG_FLASH_START_INDEX)
        || (((uint32_t)(memoryAddress_20bits) + memorySize) > (uint32_t)LDR_PROG_FLASH_END_INDEX)) {
    #if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
    #endif /* LDR_HAS_NEGATIVE_RESPONSES */
        return;
    } else if (memorySize > LDR_MAX_FLASH_DATA_BYTE_SIZE) {
        /* do not report error at this stage, but limit the memorySize to max allowed instead.
         * This to inform the master of the max allowed size instead of just throwing an error
         * and leaving master in the dark about the allowed memorysize.
         * This behavior is conform the Mulan2 implementation
         */
        memorySize = LDR_MAX_FLASH_DATA_BYTE_SIZE;
    } else {
        /* To continue with action for read/erase/write */
    }
#endif /* LDR_HAS_STUB_READ_MEM */

    if (readFlash == true) { /* Read Flash */
#if !LDR_HAS_FLASH_READ_OUT_PROTECTION
        LDR_DEBUG_OUTPUT("LDR: Read Flash operation");
        ldr_BuildPositiveResponseReadMem(transfer, &memoryAddress_20bits, memorySize);
#else
        LDR_DEBUG_OUTPUT("LDR: Read Flash operation is not allowed");
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_INCORRECT_OPERATION);
#endif
#endif
    } else { /* Write Flash or Erase Flash */
        LDR_DEBUG_OUTPUT("LDR: Write / Erase Flash operation");
        if ((memoryAddress_20bits % LDR_FLASH_PAGE_BYTE_SIZE) != 0u) {
#if LDR_HAS_NEGATIVE_RESPONSES
            ldr_BuildNegativeResponse(transfer, MLX16ERROR_START_ADDRESS_NOT_PAGE_ALIGNED);
#endif
            return;
        }

        ldrMe->memoryAddress = memoryAddress_20bits;
        ldrMe->memorySize = memorySize;
        ldrMe->selectedBlock = LB_FLASH;

        if (eraseFlash == true) { /* Erase */
            LDR_DEBUG_OUTPUT("LDR: Handler ldr_FlashErase for one sector");
            pldrRamFunctions->ldr_FlashErase(&memoryAddress_20bits);
            ldr_BuildPositiveResponseWriteMem(transfer, true);
        } else { /* Write */
            /* flash needs page alignment */
            if ((memorySize % LDR_FLASH_PAGE_BYTE_SIZE) != 0u) {
#if LDR_HAS_NEGATIVE_RESPONSES
                ldr_BuildNegativeResponse(transfer, MLX16ERROR_WRITE_EXCEEDS_PAGE);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
            } else {
                ldr_BuildPositiveResponseWriteMem(transfer, true);
            }
        }
    }
}

#if LDR_HAS_STUB_READ_MEM
static void ldr_BuildPositiveResponseReadMemStubbed(LINDiagTransfer_t* transfer)
{
    LINDiagResponse_t *response = &(transfer->response);
    response->respSId = LDR_RES_DATA_DUMP;
    response->data[0] = LDR_STATE;
    response->dataLen = 1;
}
#endif /* LDR_HAS_STUB_READ_MEM */

#endif /* LDT_HAS_FLASH_FUNCTIONS */

/* EOF */

