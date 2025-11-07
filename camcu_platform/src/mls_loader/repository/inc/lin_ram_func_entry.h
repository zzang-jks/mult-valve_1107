/**
 * @file ram_func_entry.h
 * @brief The structure with pointers to the LIN Loader functions
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
#ifndef RAM_FUNC_ENTRY_H_
#define RAM_FUNC_ENTRY_H_

#include <stdint.h>
#include <stdbool.h>
#include <lin_ldr_config.h>


/*-----------------------------------------------------------------------------
 * Typedefs					            (scope: global)
 */

typedef struct {
    /*!< Read the memory
     * @param[in] dst_add_16bits    pointer to destination memory block
     * @param[in] p_src_add_20bits  pointer to source memory block address
     * @param[in] data_buffer       memory block size
     * @return  operation result
     */
    bool (*ldr_ReadMem)(uint8_t *dst_add_16bits, const uint32_t *p_src_add_20bits, uint16_t data_size);

    /*!< Get the timeout for any write operations
     * @return  timeout in ms
     */
    uint8_t (*ldr_GetWriteTimeout_ms)(void);

    /*!< Write the RAM
     * @param[in] p_dest_addr_20bits    pointer to destination memory block address which is word aligned
     * @param[in] data_size             memory block size
     * @param[in] data_buffer           pointer to source memory block
     * @return  operation result
     */
    bool (*ldr_RamWrite)(const uint32_t *p_dest_addr_20bits, uint16_t data_size, const uint8_t *data_buffer);

    /*!< Write the EEPROM
     * @param[in] p_dest_addr_20bits    pointer to destination memory block address which is word aligned
     * @param[in] data_size             memory block size
     * @param[in] data_buffer           pointer to source memory block
     * @return  operation result
     */
    bool (*ldr_EepromWrite)(const uint32_t *p_dest_addr_20bits, uint16_t data_size, const uint8_t *data_buffer);

    /*!< Write the Flash
     * @param[in] p_dest_addr_20bits    pointer to destination memory block address which is word aligned
     * @param[in] data_size             memory block size
     * @param[in] data_buffer           pointer to source memory block
     * @return  operation result
     */
    bool (*ldr_FlashWrite)(const uint32_t *p_dest_addr_20bits, uint16_t data_size, const uint8_t *data_buffer);

    /*!< Calculates CRC on memory block
     * Optimized implementation of CRC16 using the CCITT polynomial 1021
     * (X^16 + X^12 + X^5 + 1)
     * The function is based on 'mathlib.h -> crc_ccit'.
     * @note: the 16-bit addressed memory is supported only.
     * @param[in] src   pointer to source memory block address
     * @param[in] size  memory block size
     * @return  CRC calculated on specified addresses
     */
    uint16_t (*ldr_pe_CalculateCRC16)(const uint32_t *memoryAddress_20bits, uint16_t size);

    /*!< Request entering to LIN programming mode.
     * If application is available, it can permit or forbid the LIN programming mode entering.
     * @return  true if LIN programming mode entering is allowed; false otherwise
     */
    bool (*ldr_ApplicationStop)(void);

    /*!< Request enter LIN programming mode.
     */
    void (*ldr_EnterProgMode)(void);

    /*!< Build positive response on enter LIN programming mode.
     * @param[out] transfer   pointer to LIN TL output buffer
     */
    void (*ldr_BuildPositiveResponseEnterProg)(LINDiagTransfer_t* transfer);

    /*!< Initiate full Flash erase.
     * @param[in] pageNumber   the pointer to Flash destination address to erase; the 0xFFFFFFFF destination address is used to erase full Flash.
     */
    void (*ldr_FlashErase)(const uint32_t *p_dest_addr_20bits);

} ldr_RamFunctions_t;


/*-----------------------------------------------------------------------------
 * Exported function prototypes         (scope: global)
 */

extern LDR_NODP_ATTR ldr_RamFunctions_t *pldrRamFunctions;

#endif /* RAM_FUNC_ENTRY_H_ */

