/**
 * @file protocol_extension.h
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
#ifndef PROTOCOL_EXTENSION_H_
#define PROTOCOL_EXTENSION_H_

#include <stdint.h>
#include "mls_api.h"

/*-----------------------------------------------------------------------------
 * Typedefs					            (scope: module-local)
 */

/** Protocol extension commands */
/** This commands available with 'CMD_PROT_EXTENSION' command only */
typedef enum {
    peReadFlashModify   = 0x00u,
    peMarginModify      = 0x01u, /* not supported (the margin read functionality is possible by ldr_ReadMem() handler overwrite) */
    peFullFlashErase    = 0xF0u
} ProtocolExtensionCode_t;

/** ReadFlash modify commands */
/** This modes available with 'peReadFlashModify' code only */
typedef enum {
    rfmNormal           = 0x00u,
    rfmCrcCalc          = 0x01u
} ReadFlashModify_t;

/** Flash Margin modify commands */
/** This modes available with 'peMarginModify' code only */
typedef enum {
    mmMarginSetOffset   = 0x00u /* not supported */
} MarginModify_t;


/*-----------------------------------------------------------------------------
 * Constants and macros                 (scope: module-local)
 */

#define FLASH_ERASE_ALL_SECTORS   (0xFFFFFFFFUL)


/*-----------------------------------------------------------------------------
 * Exported function prototypes         (scope: global)
 */

extern void pe_Handle(LINDiagTransfer_t* transfer);
extern void pe_BuildPositiveResponseReadCRC(
    LINDiagTransfer_t* transfer, uint32_t *memoryAddress_20bits, uint16_t size);


#endif /* PROTOCOL_EXTENSION_H_ */
/* EOF */
