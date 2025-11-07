/**
 * @file lin_ldr.h
 * @brief Interface definitions for eol layer. Token: ldr
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

#ifndef LDR_H_
#define LDR_H_

#include "lin_ldr_config.h"
#include "protocol_extension.h"
#include "lin_ram_func_entry.h"


/*-----------------------------------------------------------------------------
 * Constants and macros                 (scope: module-local)
 */

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

/* Position of parameter sub function in data array */
#define LDR_DATAI_MLXCOMMAND 0


/*-----------------------------------------------------------------------------
 * Typedefs                             (scope: global)
 */

/** Mxl16 Errors */
typedef enum {
    MLX16ERROR_WRITE_EXCEEDS_PAGE = 0x0,
    MLX16ERROR_TABLE_DOESNT_EXIST = 0x1,
    MLX16ERROR_TOO_MANY_DATA_FRAME_RECEIVED = 0x2,
    MLX16ERROR_FRAME_COUNTER_ERROR = 0x3,
    MLX16ERROR_UNABLE_TO_ENTER_FAST_PROTOCOL_MODE = 0x4,
    MLX16ERROR_WRITING_TO_FLASH_EEPROM_FAILED = 0x5,
    MLX16ERROR_OUT_OF_BOUNDS = 0x6, //TODO: Document
    MLX16ERROR_START_ADDRESS_NOT_PAGE_ALIGNED = 0x7, //TODO: Document
    MLX16ERROR_INVALID_PCI_BYTE = 0xB,
    MLX16ERROR_TOO_MANY_CONTINUOUS_FRAME = 0xC,
    MLX16ERROR_INCORRECT_OPERATION = 0xD,
    MLX16ERROR_NOT_IN_PROGRAMMING_MODE = 0xE,
    MLX16ERROR_NO_ERROR = 0xF
} LdrMlx16Error_t;

typedef enum {
    LB_EEPROM,
    LB_FLASH,
    LB_RAM,
    LB_NONE
} LogicalBlocks_t;

typedef enum {
    /* MLXEOL Session only */
    LDR_REQ_READ_BY_ID = 0xB2,
    LDR_REQ_DATA_DUMP = 0xB4,
} LdrRequestSId_t;

typedef enum {
    /* MLXEOL Session only */
    LDR_RES_READ_BY_ID = 0xB2+0x40,
    LDR_RES_DATA_DUMP = 0xB4+0x40,
} LdrResponseSId_t;

typedef struct PACKED {
    uint8_t peCode;
    uint8_t peValue;
} ProtocolExtension_t;

/** Bootloader commands */
typedef enum {
    CMD_RESTART =           0xC1,
    CMD_EXIT =              0x42, /* not supported */
    CMD_STATUS =            0x80,
    CMD_SET_KEY =           0x14, /* not supported */

    CMD_FAST_PROTOCOL =     0x03,
    CMD_SLOW_PROTOCOL =     0xC4, /* not supported */

    CMD_DATA =              0xD3,
    CMD_PROT_EXTENSION =    0xD6,

#if LDT_HAS_FLASH_FUNCTIONS
    CMD_ERASE_FLASH =       0x55,
    CMD_WRITE_FLASH =       0x20, /* 4 lowbits are used as AddressBits of the 20Bit Address */
    CMD_READ_FLASH =        0x30, /* 4 lowbits are used as AddressBits of the 20Bit Address */
#endif

#if LDT_HAS_EEPROM_FUNCTIONS
    CMD_WRITE_EEPROM =      0x47,
    CMD_READ_EEPROM =       0x08,
    CMD_ERASE_EEPROM =      0x49, /* not supported */
    CMD_ERASE_ALL_EEPROM =  0xCA, /* not supported */
#endif

#if LDT_HAS_NVRAM_FUNCTIONS
    CMD_RESTORE_NVRAM =     0x8B,
    CMD_SAVE_NVRAM =        0x4C,
#endif

#if LDT_HAS_RAM_FUNCTIONS
    CMD_WRITE_RAM =         0x0D,
    CMD_READ_RAM =          0x8E,
#endif

#if !defined(LDT_DISABLE_RAM_EXEC_FUNCTIONS) || (LDT_DISABLE_RAM_EXEC_FUNCTIONS == 0)
    CMD_EXECUTE_RAM =       0xCF,
#endif

#if LDT_HAS_TABLE_FUNCTIONS
    CMD_EXIST_TABLE =       0x50, /* not supported */
    CMD_WRITE_TABLE =       0x11, /* not supported */
    CMD_READ_TABLE =        0x92,
#endif

} Command_t;

typedef struct {
    Command_t commandID;
    bool MlxLinLdrSession;
    ProtocolExtension_t protExt;
    uint32_t memoryAddress;
    uint16_t memorySize;
    LogicalBlocks_t selectedBlock;
} LDR_t;

typedef void (*ldr_ExecuteRam_t)(void);


/*-----------------------------------------------------------------------------
 * Declaration of variables             (scope: module-local)
 */

extern LDR_NODP_ATTR LDR_t* ldrMe;


/*-----------------------------------------------------------------------------
 * Exported function prototypes         (scope: global)
 */

extern void ldr_InitRamTable (ldr_RamFunctions_t *resRamTable);
extern void ldr_Init(LDR_t *privateVariables);


/*-----------------------------------------------------------------------------
 * Exported function prototypes         (scope: module-local)
 */

extern void ldr_BuildNegativeResponse(LINDiagTransfer_t *transfer, LdrMlx16Error_t mlx16Error);
extern void ldr_BuildPositiveResponseMlxCmd(LINDiagTransfer_t* transfer);

#if LDT_HAS_FLASH_FUNCTIONS || LDT_HAS_NVRAM_FUNCTIONS || LDT_HAS_EEPROM_FUNCTIONS || LDT_HAS_RAM_FUNCTIONS
extern void ldr_BuildPositiveResponseWriteMem(LINDiagTransfer_t* transfer, bool isWriteTO);
extern void ldr_BuildPositiveResponseReadMem(
    LINDiagTransfer_t* transfer, uint32_t *p_src_add_20bits, uint16_t memorySize);
#endif
extern bool ldr_HandleEnterProgMode(LINDiagTransfer_t* transfer);

/*
 * These functions shall be implemented on the project / platform side
 */

/* The MLX16 system functions */
extern void ldr_ResetMlx16(void);

#if LDT_HAS_NVRAM_FUNCTIONS
/* The NVRAM functions */
extern void ldr_NvramLoadAll(void);
extern void ldr_NvramSaveAll(void);
#endif

#endif /* LDR_H_ */
/* EOF */
