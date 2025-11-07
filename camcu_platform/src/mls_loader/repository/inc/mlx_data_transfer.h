/**
 * @file mlx_data_transfer.h
 * @brief Interface definitions for command data. Token: data
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

#ifndef MLX_DATA_TRANSFER_H_
#define MLX_DATA_TRANSFER_H_

#include "lin_ldr.h"

/*-----------------------------------------------------------------------------
 * Exported function prototypes         (scope: global)
 */

#if LDT_HAS_FLASH_FUNCTIONS || LDT_HAS_NVRAM_FUNCTIONS || LDT_HAS_EEPROM_FUNCTIONS || LDT_HAS_RAM_FUNCTIONS
extern void data_Handle(LINDiagTransfer_t* transfer);
#endif


#endif /* MLX_DATA_TRANSFER_H_ */
/* EOF */
