/**
 * @file lin_ldr_config.h
 * @brief Template for LIN Loader module configurations
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

#ifndef LIN_LDR_CONFIG_H_
#define LIN_LDR_CONFIG_H_


/*
 * Note: This is the template file is used only to achieve standalone build of the MLX LIN LDR module!
 */

#ifndef __ASSEMBLER__
#include <stdint.h>

/* Usual headers for the MLX LIN API */
#include "lin.h"
#include "lin_diag_transfer.h"
#include "lin_event_table.h"

/* Memory functions */
//#include "memtools.h"

//#include "syslib.h"
//#include "plib.h"
//#include "memtools.h"
//#include "eeprom.h"
//#include "flash.h"
//#include "pltf_version.h"


/*-----------------------------------------------------------------------------
 * Constants and macros                 (scope: module-local)
 */

/*
 * Options
 */

#define __MLX_LDR_VERSION_MAJOR__ 2
#define __MLX_LDR_VERSION_MINOR__ 0
#define __MLX_LDR_VERSION_REVISION__ 0
#define __MLX_LDR_VERSION_CUSTOMER_BUILD__ 0

/* Flash sector byte size */
#define FLASH_SECTOR_BYTE_SIZE 2048

/* Flash page byte size */
#define LDR_FLASH_PAGE_BYTE_SIZE 128 //TODO: Standardize with MLX LIN API

/* Flash page erase value */
#define DATA_FLASH_ERASE_VALUE 0

/* Only respond with single frame as required by EOL flash process */
#define LDR_HAS_STUB_READ_MEM 0

/* Support for EEPROM programming messages */
#define LDT_HAS_EEPROM_FUNCTIONS 1

/* Optimization: don't provide negative responses */
#define LDR_HAS_NEGATIVE_RESPONSES 1

/* Protocol extension functionality */
#define LDR_HAS_PROTOCOL_EXTENSION 1

/* Max flexible FAST baudrate [Kbaud] */
#define LDR_MAX_FLEX_FAST_BAUDRATE_K (ML_FAST_BAUDRATE / 1000)

/* Parity check for read/write commands */
#define LDR_HAS_PARITY_CHECK 1

/** Disable Flash read out protection */
#define LDR_HAS_FLASH_READ_OUT_PROTECTION 0

/** Disable check LIN written FLASH size in consecutive status request */
#define LDT_HAS_GET_STATUS_WRITE_CHECK 0

/*
 * Constants
 */

#define LDR_EEPROM_PAGE_BYTE_SIZE 128u

/*
 * EEPROM area definitions
 */
#define EEPROM1_START_INDEX              &_ep_start
#define LDR_PROG_EEPROM_END_INDEX                 &_ep_end

/*
 * Application area definitions
 */
#define LDR_PROG_FLASH_START_INDEX   &_stext
#define LDR_PROG_FLASH_END_INDEX     &_etext


/*-----------------------------------------------------------------------------
 * Exported linker script symbols
 */

extern uint16_t _ep_start;
extern uint16_t _ep_end;

extern uint16_t _stext;
extern uint16_t _etext;

#endif /* __ASSEMBLER__ */

#endif /* LIN_CONFIG_H_ */
