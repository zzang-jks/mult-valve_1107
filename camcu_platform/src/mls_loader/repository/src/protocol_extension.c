/**
 * @file protocol_extension.c
 * @brief Protocol extension functionality. Token: pe
 * @internal
 *
 * @copyright (C) 2017 Melexis N.V.
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


#if LDR_HAS_PROTOCOL_EXTENSION == 1

/*-----------------------------------------------------------------------------
 * Constants and macros                 (scope: module-local)
 */

#define BITS_IN_BYTE            (8)


/*-----------------------------------------------------------------------------
 * Function prototypes                  (scope: module-local)
 */


/*-----------------------------------------------------------------------------
 * Function implementation
 */

void pe_Handle(LINDiagTransfer_t* transfer)
{
    ldrMe->protExt.peCode = transfer->request.data[LDR_DATAI_MLXCOMMAND+1];
    ldrMe->protExt.peValue = transfer->request.data[LDR_DATAI_MLXCOMMAND+2];

    if (ldrMe->protExt.peCode == peFullFlashErase) {
        uint32_t dest_addr_20bits = FLASH_ERASE_ALL_SECTORS;
        LDR_DEBUG_OUTPUT("LDR: handler ldr_FlashErase for all sectors");
        pldrRamFunctions->ldr_FlashErase(&dest_addr_20bits);
    }

    /* TODO: Document that full Flash erase message returns the status */
    ldr_BuildPositiveResponseMlxCmd(transfer);
}


void pe_BuildPositiveResponseReadCRC(LINDiagTransfer_t* transfer, uint32_t *memoryAddress_20bits, uint16_t size)
{
    LDR_DEBUG_OUTPUT("LDR: handler ldr_pe_CalculateCRC16");
    uint16_t mem_block_crc = pldrRamFunctions->ldr_pe_CalculateCRC16(memoryAddress_20bits, size);

    /* ...wait... */

    LINDiagResponse_t *response = &(transfer->response);
    response->respSId = LDR_RES_DATA_DUMP;
    response->data[0] = 0xFF;                                       /* reserved */
    response->data[1] = (uint8_t)(mem_block_crc >> BITS_IN_BYTE);   /* MSB of CRC */
    response->data[2] = (uint8_t)mem_block_crc;                     /* LSB of CRC */
    response->data[3] = (uint8_t)(size >> BITS_IN_BYTE);            /* MSB of size */
    response->data[4] = (uint8_t)size;                              /* MSB of size */
    response->dataLen = 5;
}

#endif /* LDR_HAS_PROTOCOL_EXTENSION == 1 */

/* EOF */
