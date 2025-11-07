/**
 * @file
 * @brief application configuration of the unirom library
 * @internal
 *
 * @copyright (C) 2021 Melexis N.V.
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
 * @ingroup application
 *
 * @details This file configuration of the unirom library.
 */

#ifndef UNIROM_CONFIG_H_
#define UNIROM_CONFIG_H_

/** one block struct */
typedef struct
{
    uint8_t crc8;       /**< unique pattern to protect the memory */
    uint8_t payload[7]; /**< payload */
} page_t;

/** user config struct */
typedef struct user_pattern
{
    page_t page[3];
} user_pattern_t;

#endif /* UNIROM_CONFIG_H_ */
