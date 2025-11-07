/**
 * @file
 * @brief The unirom library.
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
 * This file contains the implementation of the unirom library.
 */

#include <stdint.h>
#include <stdbool.h>
#include <syslib.h>
#include <string.h>
#include <eeprom_drv.h>
#include <lib_wdg.h>
#include <mem_checks.h>
#include "unirom.h"

/* ---------------------------------------------
 * Local Defines
 * --------------------------------------------- */

/** start address of the user eeprom */
#define NV_ADDR_PATTERN ((uint16_t)EEPROM_START + 0x40u)

/** eeprom write key */
#define EE_WRITE_KEY 0x07u


/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */

/** complete RAM copy */
static user_pattern_t l_ramCopy __attribute__((aligned(2)));


/* ---------------------------------------------
 * Local Functions
 * --------------------------------------------- */

static bool _pageVerify(page_t* config, uint16_t * address);
static void _updateCRC8(page_t * config);


/* ---------------------------------------------
 * Public Functions Implementation
 * --------------------------------------------- */

void unirom_Init(void)
{}

bool unirom_LoadUserConfig(void)
{
    bool retVal = true;

    for (uint8_t page = 0; page < sizeof(user_pattern_t) / sizeof(page_t); page++)
    {
        EEPROM_ClearErrorFlags();

        uint16_t eeprom_address = NV_ADDR_PATTERN + (page * sizeof(page_t) / sizeof(uint8_t));

        /* copy one page from EEPROM to RAM */
        memcpy((void*)&l_ramCopy.page[page], (void*)eeprom_address, sizeof(page_t) / sizeof(uint8_t));

        if (EEPROM_GetErrorFlags())
        {
            retVal = false;  /* ECC error */
        }
        else
        {
            if (nvram_CalcCRC((void *)&l_ramCopy.page[page], sizeof(page_t) / sizeof(uint16_t)) != 0xFF)
            {
                retVal = false;  /* CRC error */
            }
        }
    }

    return retVal;
}

bool unirom_ResetUserConfig(const user_pattern_t * def_config)
{
    /* restore ram copy  */
    memcpy((void*)&l_ramCopy, (void*)def_config, sizeof(user_pattern_t) / sizeof(uint8_t));

    /* restore CRC8 of all pages */
    for (uint8_t page = 0u; page < sizeof(user_pattern_t) / sizeof(page_t); page++)
    {
        _updateCRC8(&l_ramCopy.page[page]);
    }

    /* store RAM to eeprom */
    return unirom_StoreUserConfig();
}

bool unirom_StoreUserConfig(void)
{
    for (uint8_t page = 0u; page < sizeof(user_pattern_t) / sizeof(page_t); page++)
    {
        uint16_t eeprom_address = NV_ADDR_PATTERN + (page * sizeof(page_t) / sizeof(uint8_t));

        /* Check if EEPROM and RAM copy are not the same */
        if (!_pageVerify(&l_ramCopy.page[page], (uint16_t *)eeprom_address))
        {
            /* write to eeprom page */
            ENTER_SECTION(ATOMIC_SYSTEM_MODE);
            EEPROM_WriteWord64_blocking(eeprom_address, (void *)&l_ramCopy.page[page], EE_WRITE_KEY);
            EXIT_SECTION();

            WDG_conditionalAwdRefresh();  /* Restart watchdog */
        }
    }

    return true;
}

bool unirom_StorePage(uint8_t page)
{
    uint16_t eeprom_address = NV_ADDR_PATTERN + (page * sizeof(page_t) / sizeof(uint8_t));

    /* Check if EEPROM and RAM copy are not the same */
    if (!_pageVerify(&l_ramCopy.page[page], (uint16_t *)eeprom_address))
    {
        /* write to eeprom page */
        ENTER_SECTION(ATOMIC_SYSTEM_MODE);
        EEPROM_WriteWord64_blocking(eeprom_address, (void*)&l_ramCopy.page[page], EE_WRITE_KEY);
        EXIT_SECTION();
    }

    return true;
}

bool unirom_WriteToPage(uint8_t page, uint8_t index, uint8_t data)
{
    bool retVal = false;

    if (l_ramCopy.page[page].payload[index] != data)  /* is new data different ? */
    {
        l_ramCopy.page[page].payload[index] = data;  /* write the data to the ram copy */

        _updateCRC8(&l_ramCopy.page[page]);  /* update CRC8 of that page */

        retVal = true;
    }

    return retVal;
}

bool unirom_ReadFromPage(uint8_t page, uint8_t index, uint8_t* data)
{
    bool retVal = false;

    if (nvram_CalcCRC((void*)l_ramCopy.page, sizeof(page_t) / sizeof(uint16_t)) == 0xFF)  /* check crc of the page */
    {
        *data = l_ramCopy.page[page].payload[index];  /* read from ram copy */
        retVal = true;
    }
    return retVal;
}

bool unirom_WritePage(uint8_t page, uint8_t* data, uint8_t len)
{
    bool retVal = false;

    if (len <= 7u)
    {
        if (memcmp((void *)l_ramCopy.page[page].payload, (void *)data, len) != 0)  /* are the new data different ? */
        {
            memcpy((void*)&l_ramCopy.page[page].payload[0], (void *)data, len);  /* write the data to the ram copy */
            memset((void*)&l_ramCopy.page[page].payload[len], 0, 7u - len);  /* clear non-used data with 0 */

            _updateCRC8(&l_ramCopy.page[page]);  /* update CRC8 of that page */

            retVal = true;
        }
    }

    return retVal;
}

bool unirom_ReadPage(uint8_t page, uint8_t* data, uint8_t len)
{
    bool retVal = false;

    if (len <= 7u)
    {
        if (nvram_CalcCRC((void*)&l_ramCopy.page[page], sizeof(page_t) / sizeof(uint16_t)) == 0xFF)  /* check crc of the page */
        {
            memcpy((void*)data, (void *)l_ramCopy.page[page].payload, len);  /* read from ram copy */

            retVal = true;
        }
    }
    return retVal;
}


/* ---------------------------
 * Local Functions Implementation
 * --------------------------- */

/**
 * @brief Compare RAM copy and EEPROM contents
 * @param[in]  config  page data in RAM
 * @param[in]  address  page data in EEPROM
 * @retval  true  in case of success
 */
static bool _pageVerify(page_t * config, uint16_t * address)
{
    return (memcmp((void *)config, (void *)address, sizeof(page_t) / sizeof(uint8_t)) == 0);
}

/**
 * @brief Calculate and update CRC8 on one page
 * @param[in]  config  page data
 */
static void _updateCRC8(page_t * config)
{
    uint16_t u16CRC;

    config->crc8 = (uint16_t)0x00;

    u16CRC = nvram_CalcCRC((void *)config, sizeof(page_t) / sizeof(uint16_t));

    config->crc8 = (uint16_t)(0xFFU - u16CRC);
}

/* EOF */
