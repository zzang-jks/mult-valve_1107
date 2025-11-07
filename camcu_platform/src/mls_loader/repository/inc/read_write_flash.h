/**
 * @file read_write_flash.h
 * @brief Interface definitions for command write flash and read flash. Token: rwf
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

#ifndef READ_WRITE_FLASH_H_
#define READ_WRITE_FLASH_H_

#include "mls_api.h"

/*-----------------------------------------------------------------------------
 * Exported function prototypes         (scope: global)
 */

extern void rwf_Handle(LINDiagTransfer_t* transfer);


#endif /* READ_WRITE_FLASH_H_ */
/* EOF */
