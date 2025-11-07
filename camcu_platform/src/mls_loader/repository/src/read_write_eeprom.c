/**
 * @file read_write_eeprom.c
 * @brief MlxEOL Read EEPROM and Write EEPROM services.  Token: rwn
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


#if LDT_HAS_EEPROM_FUNCTIONS == 1


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

void rwe_Handle(LINDiagTransfer_t* transfer)
{
    LINDiagRequest_t *request = &(transfer->request);
    uint8_t cmdID = request->data[0];
    bool readEEPROM;

    if (cmdID == CMD_WRITE_EEPROM) { /* Write */
        readEEPROM = false;
    } else { /* Read */
        readEEPROM = true;
    }

    uint16_t memoryAddress = request->data[LIN_ADDRESS_BLK1_IX];
    memoryAddress <<= BITS_IN_BYTE;
    memoryAddress |= request->data[LIN_ADDRESS_BLK0_IX];
    uint16_t memorySize = request->data[LIN_ADDRESS_REM1_IX];
    memorySize <<= BITS_IN_BYTE;
    memorySize |= request->data[LIN_ADDRESS_REM0_IX];

    if (memoryAddress < (uint16_t)LDR_PROG_EEPROM_START_INDEX) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
#endif
        return;
    }

    if (memorySize > LDR_MAX_EEPROM_DATA_BYTE_SIZE) {
#if LDR_HAS_NEGATIVE_RESPONSES
        ldr_BuildNegativeResponse(transfer, MLX16ERROR_WRITE_EXCEEDS_PAGE);
#endif
        return;
    }

    /* NOTE: the EEPROM data alignment isn't required on this stage, but depends on the ldr_EepromWrite() implementation */

    if (readEEPROM == true) {
        if ((memoryAddress + memorySize) > (uint16_t)LDR_PROG_EEPROM_END_INDEX) {
#if LDR_HAS_NEGATIVE_RESPONSES
            ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
#endif
            return;
        }

        /* The EEPROM is constrained by MLX EOL Loader specifications to be into the word addressed memory */
        uint32_t src_20bit = (uint32_t)memoryAddress;
        ldr_BuildPositiveResponseReadMem(transfer, &src_20bit, memorySize);
    } else { /* Write EEPROM */

        if ((memoryAddress + memorySize) > (uint16_t)LDF_PROG_EEPROM_WRITE_PROT_START_INDEX) {
#if LDR_HAS_NEGATIVE_RESPONSES
            ldr_BuildNegativeResponse(transfer, MLX16ERROR_OUT_OF_BOUNDS);
#endif
            return;
        }

        ldrMe->memoryAddress = memoryAddress;
        ldrMe->memorySize = memorySize;
        ldrMe->selectedBlock = LB_EEPROM;
        ldr_BuildPositiveResponseWriteMem(transfer, true);
    }
}

#endif /* LDT_HAS_EEPROM_FUNCTIONS */

/* EOF */
