/**
 * @file
 * @brief The unirom library definitions.
 * @internal
 *
 * @copyright (C) 2017-2022 Melexis N.V.
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
 * @ingroup libraries
 *
 * @details
 * This file contains the definitions of the unirom library.
 */

#ifndef UNIROM_H_
    #define UNIROM_H_

#include <plib.h>
#include <stdbool.h>
#include "unirom_config.h"

/* ---------------------------
 * Public Function Definitions
 * --------------------------- */

/**
 * Initialize the unirom library
 */
void unirom_Init(void);

/**
 * Load User configuration
 *
 * This function loads the User configuration from the eeprom and copy to RAM
 * @retval  true  in case of success, false if any of the data in one page is corrupted
 */
bool unirom_LoadUserConfig(void);

/**
 * Load new user configuration (ie. default configuration) and store to eeprom
 *
 * @param[in]  def_config  address of the new configuration
 * @retval  true  in case of success
 */
bool unirom_ResetUserConfig(const user_pattern_t * def_config);

/**
 * Store all pages to eeprom
 * @retval  true  in case of success
 */
bool unirom_StoreUserConfig(void);

/**
 * Store one page to eeprom
 * @param  page  identifier of the page
 * @retval  true  in case of success
 */
bool unirom_StorePage(uint8_t page);

/**
 * Write a new byte to one page
 * @param  page  identifier of the page
 * @param  index  payload index (byte)
 * @param  data  new data (byte)
 * @retval  true  in case of success
 */
bool unirom_WriteToPage(uint8_t page, uint8_t index, uint8_t data);

/**
 * Read one new byte from one page
 * @param  page  identifier of the page
 * @param  index  payload index (byte)
 * @param  data  new data (byte)
 * @retval  true  in case of success
 */
bool unirom_ReadFromPage(uint8_t page, uint8_t index, uint8_t* data);

/**
 * Write a new page
 * @param  page  identifier of the page
 * @param  data  buffer containing new data
 * @param  len  number of bytes in buffer, rest of bytes will be cleared
 * @retval  true  in case of success
 */
bool unirom_WritePage(uint8_t page, uint8_t* data, uint8_t len);

/**
 * Read a full page
 * @param  page  identifier of the page
 * @param  data  buffer containing new data
 * @param  len  number of bytes in buffer
 * @retval  true  in case of success
 */
bool unirom_ReadPage(uint8_t page, uint8_t* data, uint8_t len);

#endif  /* UNIROM_H_ */

/* EOF */
