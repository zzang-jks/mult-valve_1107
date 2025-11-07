/**
 * @file
 * @brief The application eeprom module.
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
 * @details This file contains the implementations of the application eeprom module.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <lin_api.h>
#include <unirom.h>
#include "eeprom_app.h"

/* ---------------------------------------------
 * Local Constants
 * --------------------------------------------- */

/** eeprom default values */
const user_pattern_t eeprom_defaults =
    {
        .page[0] =
            {
                .crc8 = 171,
                .payload = ML_NODE_CONFIGURATION_INITIALIZER,
            },
        .page[1] =
            {
                .crc8 = 0xFF,
                .payload = {0},
            },
        .page[2] =
            {
                .crc8 = 0xFF,
                .payload = {0},
            }};

valve_config_t valve_gmr_data;
valve_config_t valve_diag_data;
/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */

/* ---------------------------------------------
 * Local Function Declarations
 * --------------------------------------------- */

/**
 * Module initialization
 */
bool eeprom_Init(void)
{
    bool retval = true;

    unirom_Init();

    if (!unirom_LoadUserConfig())
    {
        (void)unirom_ResetUserConfig(&eeprom_defaults);

        retval = false;
    }

    return retval;
}

/** Read lin configuration
 *
 * This function reads the lin configuration from the eeprom.
 * @param[out]  config  the read configuration array
 * @param[in]  length  the number of configuration words to read
 * @retval  true  valid configuration found in eeprom.
 * @retval  false  otherwise.
 */
bool eeprom_ReadLINconfig(uint8_t *config, uint8_t length)
{
    bool retval = false;
    retval = unirom_ReadPage(0u, config, length);

    return retval;
}

/** Store lin configuration
 *
 * This function stores the lin configuration to the eeprom.
 * @param[out]  config  the configuration array to be stored
 * @param[in]  length  the number of configuration words to store
 * @retval  true  the configuration is correctly stored
 * @retval  false  otherwise
 */
bool eeprom_StoreLINconfig(uint8_t *config, uint8_t length)
{
    bool retval = false;

    if (length <= 7)
    {
        (void)unirom_WritePage(0u, config, length);

        (void)unirom_StoreUserConfig();

        retval = true;
    }

    return retval;
}

/** Read position
 *
 * This function reads the position from the eeprom.
 * @param[out]  position  the position
 * @retval  true  valid position found in eeprom.
 * @retval  false  otherwise.
 */
bool eeprom_ReadValveConfig(valve_config_t *config)
{
    bool retval = false;
    uint8_t bytes[sizeof(valve_config_t)];
    (void)unirom_ReadPage(1u, &bytes[0], sizeof(valve_config_t));

    config->E1DATA0 = (uint16_t)(bytes[0] + ((uint16_t)bytes[1] << 8));
    config->E1DATA1 = (uint16_t)(bytes[2] + ((uint16_t)bytes[3] << 8));
    config->E1DATA2 = (uint16_t)(bytes[4] + ((uint16_t)bytes[5] << 8));

    retval = true;

    return retval;
}

/** Store valve configuration
 *
 * This function stores the window configuration to the eeprom.
 * @param[out]  config  the configuration array to be stored
 * @retval  true  the configuration is correctly stored
 * @retval  false  otherwise
 */
bool eeprom_WriteValveConfig(valve_config_t *config)
{
    bool retval = true;
    uint8_t bytes[sizeof(valve_config_t)];

    bytes[0] = (uint8_t)((uint16_t)config->E1DATA0 & 0xFF);
    bytes[1] = (uint8_t)(((uint16_t)config->E1DATA0 >> 8) & 0xFF);
    bytes[2] = (uint8_t)((uint16_t)config->E1DATA1 & 0xFF);
    bytes[3] = (uint8_t)(((uint16_t)config->E1DATA1 >> 8) & 0xFF);
    bytes[4] = (uint8_t)((uint16_t)config->E1DATA2 & 0xFF);
    bytes[5] = (uint8_t)(((uint16_t)config->E1DATA2 >> 8) & 0xFF);

    (void)unirom_WritePage(1u, &bytes[0], sizeof(valve_config_t));
    return retval;
}

/** Read diag data
 *
 * @retval  true  valid position found in eeprom.
 * @retval  false  otherwise.
 */
bool eeprom_ReadDiagConfig(valve_config_t *config)
{
    bool retval = false;
    uint8_t bytes[sizeof(valve_config_t)];
    (void)unirom_ReadPage(2u, &bytes[0], sizeof(valve_config_t));

    config->E1DATA0 = (uint16_t)(bytes[0] + ((uint16_t)bytes[1] << 8));
    config->E1DATA1 = (uint16_t)(bytes[2] + ((uint16_t)bytes[3] << 8));
    config->E1DATA2 = (uint16_t)(bytes[4] + ((uint16_t)bytes[5] << 8));

    retval = true;

    return retval;
}
bool eeprom_WriteDiagConfig(valve_config_t *config)
{
    bool retval = true;
    uint8_t bytes[sizeof(valve_config_t)];

    bytes[0] = (uint8_t)((uint16_t)config->E1DATA0 & 0xFF);
    bytes[1] = (uint8_t)(((uint16_t)config->E1DATA0 >> 8) & 0xFF);
    bytes[2] = (uint8_t)((uint16_t)config->E1DATA1 & 0xFF);
    bytes[3] = (uint8_t)(((uint16_t)config->E1DATA1 >> 8) & 0xFF);
    bytes[4] = (uint8_t)((uint16_t)config->E1DATA2 & 0xFF);
    bytes[5] = (uint8_t)(((uint16_t)config->E1DATA2 >> 8) & 0xFF);

    (void)unirom_WritePage(2u, &bytes[0], sizeof(valve_config_t));
    return retval;
}
void eeprom_StoreUserDataConfig(uint16_t index)
{
    if (index == 1)
    {
        (void)unirom_StorePage(1u);
    }
    else if (index == 2)
    {
        (void)unirom_StorePage(2u);
    }
    else
    {
    }
}
void valve_gmr_write(uint16_t data1, uint16_t data2, uint16_t data3)
{
    valve_gmr_data.E1DATA0 = data1;
    valve_gmr_data.E1DATA1 = data2;
    valve_gmr_data.E1DATA2 = data3;
    (void)eeprom_WriteValveConfig(&valve_gmr_data);
    eeprom_StoreUserDataConfig(1);
}
void valve_diag_write(uint16_t data1, uint16_t data2, uint16_t data3)
{
    valve_diag_data.E1DATA0 = data1;
    valve_diag_data.E1DATA1 = data2;
    valve_diag_data.E1DATA2 = data3;
    (void)eeprom_WriteDiagConfig(&valve_diag_data);
    eeprom_StoreUserDataConfig(2);
}
/* EOF */
