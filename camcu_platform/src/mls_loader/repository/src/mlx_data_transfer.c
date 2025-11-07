/**
 * @file mlx_data_transfer.c
 * @brief Melexis command data from writing data blocks into EEPROM or Flash. Token: data
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

#include <mlx_data_transfer.h>


/*-----------------------------------------------------------------------------
 * Constants and macros                 (scope: module-local)
 */

#define LIN_ADDRESS_BLK1_IX (1)         /**< offset in message address */
#define LIN_ADDRESS_BLK0_IX (2)         /**< offset in message address */
#define LIN_ADDRESS_REM1_IX (3)         /**< offset in message address */
#define LIN_ADDRESS_REM0_IX (4)         /**< offset in message address */
#define BITS_IN_BYTE        (8)


/*-----------------------------------------------------------------------------
 * Function prototypes                  (scope: module-local)
 */

#if LDT_HAS_FLASH_FUNCTIONS
static void data_HandleWriteFlash(LINDiagTransfer_t* transfer);
#endif

#if LDT_HAS_EEPROM_FUNCTIONS
static void data_HandleWriteEeprom(LINDiagTransfer_t* transfer);
#endif

#if LDT_HAS_RAM_FUNCTIONS
static void data_HandleWriteRam(LINDiagTransfer_t* transfer);
#endif


/*-----------------------------------------------------------------------------
 * Function implementation
 */

#if LDT_HAS_FLASH_FUNCTIONS || LDT_HAS_NVRAM_FUNCTIONS || LDT_HAS_EEPROM_FUNCTIONS || LDT_HAS_RAM_FUNCTIONS
void data_Handle(LINDiagTransfer_t* transfer)
{
    switch (ldrMe->selectedBlock) {
#if LDT_HAS_FLASH_FUNCTIONS
        case LB_FLASH:
            LDR_DEBUG_OUTPUT("LDR: Write Flash?");
            data_HandleWriteFlash(transfer);
            break;
#endif /* LDT_HAS_FLASH_FUNCTIONS */
#if LDT_HAS_EEPROM_FUNCTIONS
        case LB_EEPROM:
            LDR_DEBUG_OUTPUT("LDR: Write EEPROM?");
            data_HandleWriteEeprom(transfer);
            break;
#endif /* LDT_HAS_EEPROM_FUNCTIONS */
#if LDT_HAS_RAM_FUNCTIONS
        case LB_RAM:
            LDR_DEBUG_OUTPUT("LDR: Write RAM?");
            data_HandleWriteRam(transfer);
            break;
#endif /* LDT_HAS_RAM_FUNCTIONS */
        default:
            break;
    }
}
#endif


#if LDT_HAS_EEPROM_FUNCTIONS
static void data_HandleWriteEeprom(LINDiagTransfer_t* transfer)
{
    LINDiagRequest_t *request = &(transfer->request);
    if(request->dataLen != (ldrMe->memorySize+1)) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
        return;
    }

    /* The dataLen (not (ldrMe->memorySize+1)) is used even the LDR_HAS_NEGATIVE_RESPONSES is disabled
     * NOTE: the data[0] is used for MLX command storing -> use dataLen-1; use pointer to data[1] as an input data */
    LDR_DEBUG_OUTPUT("LDR: handler ldr_EepromWrite");
    if (pldrRamFunctions->ldr_EepromWrite(&(ldrMe->memoryAddress), (request->dataLen-1),
                                          &(request->data[1])) == false) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_WRITING_TO_FLASH_EEPROM_FAILED);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
        return;
    } else {
        ldr_BuildPositiveResponseMlxCmd(transfer);
    }
}
#endif /* LDT_HAS_EEPROM_FUNCTIONS */


#if LDT_HAS_FLASH_FUNCTIONS
static void data_HandleWriteFlash(LINDiagTransfer_t* transfer)
{
    LINDiagRequest_t *request = &(transfer->request);
    if(request->dataLen != (ldrMe->memorySize+1)) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
        return;
    }

    /* The dataLen (not (ldrMe->memorySize+1)) is used even the LDR_HAS_NEGATIVE_RESPONSES is disabled
     * NOTE: the data[0] is used for MLX command storing -> use dataLen-1; use pointer to data[1] as an input data */
    LDR_DEBUG_OUTPUT("LDR: handler ldr_FlashWrite");
    if (pldrRamFunctions->ldr_FlashWrite(&(ldrMe->memoryAddress), (request->dataLen-1), &(request->data[1])) == false) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_WRITING_TO_FLASH_EEPROM_FAILED);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
        return;
    } else {
        ldr_BuildPositiveResponseMlxCmd(transfer);
    }
}
#endif /* LDT_HAS_FLASH_FUNCTIONS */


#if LDT_HAS_RAM_FUNCTIONS
static void data_HandleWriteRam(LINDiagTransfer_t* transfer)
{
    LINDiagRequest_t *request = &(transfer->request);
    if(request->dataLen != (ldrMe->memorySize+1)) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
        return;
    }

    /* The dataLen (not (ldrMe->memorySize+1)) is used even the LDR_HAS_NEGATIVE_RESPONSES is disabled
     * NOTE: the data[0] is used for MLX command storing -> use dataLen-1; use pointer to data[1] as an input data */
    LDR_DEBUG_OUTPUT("LDR: handler ldr_RamWrite");
    if (pldrRamFunctions->ldr_RamWrite(&(ldrMe->memoryAddress), (request->dataLen-1), &(request->data[1])) == false) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_WRITING_TO_FLASH_EEPROM_FAILED);
#endif /* LDR_HAS_NEGATIVE_RESPONSES */
        return;
    } else {
        ldr_BuildPositiveResponseMlxCmd(transfer);
    }
}
#endif /* LDT_HAS_RAM_FUNCTIONS */

/* EOF */
