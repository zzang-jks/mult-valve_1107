/**
 * @file
 * @brief The application eeprom module definitions.
 * @internal
 *
 * @copyright (C) 2018-2020 Melexis N.V.
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
 * @details This file contains the definitions of the application eeprom module.
 */

#ifndef EEPROM_APP_H_
#define EEPROM_APP_H_

#include <stdint.h>
#include <stdbool.h>

/* ---------------------------------------------
 * Public Types
 * --------------------------------------------- */

typedef struct valve_config
{
    uint16_t E1DATA0; // 240820-HMD-2
    uint16_t E1DATA1; // 240820-HMD-2
    uint16_t E1DATA2; // 240820-HMD-2
} valve_config_t;
extern valve_config_t valve_diag_data;
extern valve_config_t valve_gmr_data;
/* ---------------------------------------------
 * Public Function Declarations
 * --------------------------------------------- */

bool eeprom_Init(void);
bool eeprom_ReadLINconfig(uint8_t *config, uint8_t length);
bool eeprom_StoreLINconfig(uint8_t *config, uint8_t length);
bool eeprom_ReadValveConfig(valve_config_t *config);
bool eeprom_WriteValveConfig(valve_config_t *config);
bool eeprom_ReadDiagConfig(valve_config_t *config);
bool eeprom_WriteDiagConfig(valve_config_t *config);
void eeprom_StoreUserDataConfig(uint16_t index);
void valve_gmr_write(uint16_t data1, uint16_t data2, uint16_t data3);
void valve_diag_write(uint16_t data1, uint16_t data2, uint16_t data3);
#endif /* EEPROM_APP_H_ */

/* EOF */
