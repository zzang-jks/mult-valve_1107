/**
 * @file
 * @brief user pattern
 *
 * @copyright (C) 2017-2021 Melexis N.V. All rights reserved.
 *
 * @section license
 * Melexis N.V. is supplying this code for use with Melexis N.V. processor based microcontrollers only.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.  MELEXIS N.V. SHALL NOT IN ANY CIRCUMSTANCES,
 * BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Melexis confidential & proprietary
 *
 * @ingroup application
 */

#ifndef UNIROM_CONFIG_H_
    #define UNIROM_CONFIG_H_

/** eeprom pattern type struct */
typedef struct
{
    uint16_t crc8;  /**< unique pattern to protect the memory */
    uint16_t len;  /**< number of words in the all user pattern struct */
} eeprom_pattern;

/** user config struct */
typedef struct user_pattern
{
    eeprom_pattern nv_pattern;
    uint16_t lin_config[12];  /**< max 12 words reserved for LIN config */
    int16_t position;
} user_pattern_t;

#endif  /* UNIROM_CONFIG_H_ */

/* EOF */
