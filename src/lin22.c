/**
 * @file
 * @brief The lin v2.2 module.
 * @internal
 *
 * @copyright (C) 2019-2020 Melexis N.V.
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
 * @details This file contains the implementations of the lin v2.2 module.
 */

#include <string.h>
#include <bist.h>
#include <plib.h>
#include <lin_api.h>
#include <lin_signals.h>
#include <fwversion.h>
#include <eeprom_drv.h>
#include <itc_helper.h>
#include <libraries_version.h>
#include <lib_softio.h>
#include <swtimer.h>
#include "adc.h"
#include <flash_defines.h>
#if (SL_HAS_SERIAL_NUMBER_CALLOUT == 1)
#include <eeprom_map.h>
#endif /* (SL_HAS_SERIAL_NUMBER_CALLOUT == 1) */
#include <mls_support.h>
#if (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1)
#include "eeprom_app.h"
#endif /* (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1) */
#include "fw_ints_prio.h"
#include "lin22.h"
#include "pwm.h"
#include "defines.h"

/* ---------------------------------------------
 * Local Defines
 * --------------------------------------------- */

/** the number of `ldt_Tick()` events to generate to the api for a complete N_As timeout (1..100) */
#define TO_N_AS_TIMEOUT_TICKS (100u * 10u)
/** the number of `ldt_Tick()` events to generate to the api for a complete N_Cr timeout (1..100) */
#define TO_N_CR_TIMEOUT_TICKS (100u * 10u)
/** period of checking the COLIN module state (in ms) */
#define COLIN_CHECK_TIMEOUT (300u * 10u)

/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */

#if (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1)
extern l_s_IfcStatus_t s_ifcStatus; /**< this is a LIN API internal variable */
#endif                              /* (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1) */
#pragma space nodp
uint8_t bLinActive = false;
uint8_t bLinTimeoutActive = false;
#ifdef APP_HAS_DEBUG
#if (DEBUG_DB_B2 == 1)
uint16_t l_u16DebugBuffer[DEBUG_BUFFER_SIZE];                                                     /* Debug buffer */
uint16_t g_u16DebugCaptureTrigger[3] = {DEBUG_BUFFER_SIZE, DEBUG_BUFFER_SIZE, DEBUG_BUFFER_SIZE}; /* Debug buffer index */
uint16_t g_u16DebugCaptureEnd[3] = {DEBUG_BUFFER_SIZE, DEBUG_BUFFER_SIZE, DEBUG_BUFFER_SIZE};
uint16_t l_u16DebugBufferSize = DEBUG_BUFFER_SIZE;
uint16_t l_u16DebugBufferElements = 1u;
uint16_t *l_pu16DebugBufferAddress[3] = {NULL, NULL, NULL};
uint16_t g_u16DebugCaptureCounter = 0u;
uint16_t g_u16DebugCaptureDivider = 0u;
#endif
#endif
#pragma space none

/* ---------------------------------------------
 * Local Functions Declaration
 * --------------------------------------------- */

__attribute__((weak)) void EVENT_GotoSleep(void)
{
}
void lin22_CheckColin(void);
bool fw_lepm_ApplicationStop(void);
#if (SL_HAS_UNKNOWN_DIAG_CALLOUT == 1)
#ifdef APP_HAS_DEBUG
bool DebugFrameHdlr(l_u8 inData[5], l_u8 *pci, l_u8 data[]);
#endif /* APP_HAS_DEBUG */
bool ld_AppDiagRequest(LINDiagTransfer_t *transfer);
#endif /* (SL_HAS_UNKNOWN_DIAG_CALLOUT == 1) */
#ifdef APP_HAS_DEBUG
#if (DEBUG_DB_B2 == 1)
void B2_exit(void);
#endif /* DEBUG_DB_B2 */
#endif

/* ---------------------------------------------
 * Public Functions Implementation
 * --------------------------------------------- */

/** Initialize and start the lin module
 *
 * This function initialize the lin module and it will assign
 * the default frame ids to the different messages.
 */
void lin22_Init(void)
{
    (void)l_sys_init();
    (void)l_ifc_init_sl1();

    uint8_t lin_cfg_len = SL_NUMBER_OF_DYNAMIC_MESSAGES + 1u;
    uint8_t lin_config[1 + SL_NUMBER_OF_DYNAMIC_MESSAGES] = SL_NODE_CONFIGURATION_INITIALIZER;

#if 0 //(SL_HAS_SAVE_CONFIGURATION_SERVICE == 1)
    (void)eeprom_ReadLINconfig(lin_config, lin_cfg_len);
    (void)ld_set_configuration(ifcSlave_1, lin_config, lin_cfg_len);
#else
    (void)ml_Disconnect();
    for (uint8_t index = 0u; index < (lin_cfg_len - 1); index++)
    {
        (void)ml_AssignFrameToMessageID(index, lin_config[1 + index]);
    }
    (void)ml_Connect();

    ml_ConfiguredNAD = lin_config[0];
#endif /* (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1) */

#if (SL_HAS_UNKNOWN_DIAG_CALLOUT == 1)
    (void)ldt_SubscribeMultipleHandler(plinEventTable->ldt_Request, ld_AppDiagRequest);
#endif /* (SL_HAS_UNKNOWN_DIAG_CALLOUT == 1) */

    /* register a timer for SL_LD_N_AS_TIMEOUT timeout */
    swtimer_register(SWTIMER_LINAS, (SL_LD_N_AS_TIMEOUT / TO_N_AS_TIMEOUT_TICKS), REPETITIVE);
    swtimer_start(SWTIMER_LINAS);

    /* register a timer for SL_LD_N_CR_TIMEOUT timeout */
    swtimer_register(SWTIMER_LINCR, (SL_LD_N_CR_TIMEOUT / TO_N_CR_TIMEOUT_TICKS), REPETITIVE);
    swtimer_start(SWTIMER_LINCR);

    /* register a timer for COLIN check timeout */
    swtimer_register(SWTIMER_CHECKCOLIN, COLIN_CHECK_TIMEOUT, REPETITIVE);
    swtimer_start(SWTIMER_CHECKCOLIN);

#ifdef APP_HAS_DEBUG
#if DEBUG_DB_B2 == 1
    adc_RegisterIRQ2(B2_exit);
#endif
#endif

    bLinActive = true;
    bLinTimeoutActive = 0;
}

/**
 * Stop LIN Communication, and stops MLX4
 */
void lin22_Stop(void)
{
    bLinActive = false;

    (void)ml_Disconnect(); /* Stop LIN receiving/transmission */
    ENTER_SECTION(ATOMIC_SYSTEM_MODE);
    ml_ResetDrv(); /* Stop MLX4 */
    IO_SET(PORT_LIN_XKEY, LIN_XKEY, 0xB2A3);
    IO_SET(PORT_LIN_XTX_CFG, LIN_XTX_OUT_SEL, 7u, LIN_OUT_SOFT, 1u);                           /* Output selection for LIN_XTX */
    IO_SET(PORT_LIN_XCFG, LIN_XCFG, IO_GET(PORT_LIN_XCFG, LIN_XCFG) | (1u << 0) | (1u << 14)); /* disable time-out protection (10-20ms) on TX output, select Tx driven from IO */
    EXIT_SECTION();
}

/** Periodic lin command handler
 *
 * This function should be called from the main loop. It will check if the
 * lin module needs attention and if so will execute the specific handling.
 */
void lin22_BackgroundHandler(void)
{

#if (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1)
    l_s_IfcStatus_t status;
    status.word = l_ifc_read_status_sl1();

    if (status.mapped.SaveConfig != 0)
    {
        uint8_t lin_cfg_len = SL_NUMBER_OF_DYNAMIC_MESSAGES + 1u;
        uint8_t lin_config[1 + SL_NUMBER_OF_DYNAMIC_MESSAGES];

        //        (void)ld_read_configuration(ifcSlave_1, lin_config, &lin_cfg_len);
        //        (void)eeprom_StoreLINconfig(lin_config, lin_cfg_len);
    }
    else
    {
        /* nothing to be done */
    }
#endif /* (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1) */

    /* Check LIN AS-timeout */
    if (swtimer_isTriggered(SWTIMER_LINAS))
    {
        ldt_Tick(100u / TO_N_AS_TIMEOUT_TICKS, 0u);
    }

    /* Check LIN CR-timeout */
    if (swtimer_isTriggered(SWTIMER_LINCR))
    {
        ldt_Tick(0u, 100u / TO_N_CR_TIMEOUT_TICKS);
    }

    if (swtimer_isTriggered(SWTIMER_CHECKCOLIN))
    {
        /* check COLIN state */
        lin22_CheckColin();
    }
}

/** Check the status of the COLIN module and handle accordingly */
void lin22_CheckColin(void)
{
    static uint8_t u8ColinErrorState = 0u;
    ml_LinState_t ColinState;

    ENTER_SECTION(ATOMIC_KEEP_MODE);
    ColinState = ml_GetState(ML_CLR_LIN_BUS_ACTIVITY);
    EXIT_SECTION();

    if (ColinState != ml_stINVALID)
    {
        /* COLIN is responding */
        if (COLIN_LINstatus.event_overflow != 0u)
        {
            /* command overflow was detected */
            ENTER_SECTION(ATOMIC_KEEP_MODE);
            (void)ml_GetState(ML_CLR_LIN_CMD_OVERFLOW);
            EXIT_SECTION();

            /* do handshake MLX16 <> COLIN */
            ml_SetSLVCMD(0x42u);
        }
    }
    else
    {
        /* COLIN response time-out */
        u8ColinErrorState++;

        if (u8ColinErrorState >= 4u)
        {
            /* COLIN has not been responding to often, lets reset the module */
            Sys_ResetCpu();
        }
    }
}

/** Request the IC to go into a sleep mode
 *
 * This function will never return, after the chip went into sleep mode
 * it will wake up again via a power on sequence.
 */
void lin22_GotoSleep(void)
{
    (void)fw_lepm_ApplicationStop(); /* stop the application */

    mlx16_enter_system_mode_keep_prio();

    builtin_mlx16_disable_interrupts(); /* disable the interrupts */

    disallowWarmReboot(); /* warm reboot is not allowed any more */

    WDG_conditionalAwdRefresh(); /* refresh the timers if they're used */

    IO_SET(SPI, STOP, 1u); /* stop the SPI despite it's running or not */

    //    IO_SET(PORT_MISC2_OUT, WU_IO_EN, 0u);           /* disable wake up on IO0 */
    IO_SET(PORT_MISC2_OUT, WU_IO_EN, 1u); /* enable wake up on IO0 */ // 250702-3 Support Sleep by IO0.
    ml_ResetDrv();                                                    /* stop the MLX4 */

    /* wait for EEPROM operations */
    while (IO_GET(EEPROM_FLASH, EE_BUSY) != 0u)
    {
    }
    /* wait for Flash operations */
    while ((IO_GET(EEPROM_FLASH, FL_STATUS) == FLASH_STATUS_PAGE_PROGRAM) ||
           (IO_GET(EEPROM_FLASH, FL_STATUS) == FLASH_STATUS_SECTOR_ERASE))
    {
    }

    /* stop MLX16 */
    __asm__("HALT\n\t");

    /* chip should reset upon LIN bus changes
     * we should never make it to here, as a backup we add a chip reset */
    Sys_ResetCpu();
}

/* ---------------------------------------------
 * Private Functions and Callbacks Implementation
 * --------------------------------------------- */

#if (SL_HAS_SERIAL_NUMBER_CALLOUT == 1)
/** Lin api event: serial number
 *
 * This function is called by the lin driver on receipt of a
 * read by id 0x00 request.
 * @param[out]  data  pointer to the data to be filled
 */
void ld_serial_number_callout(l_u8 data[4])
{
    data[0] = (l_u8)(EE_CHIP_ID_0); /* LSB */
    data[1] = (l_u8)(EE_CHIP_ID_0 >> 8);
    data[2] = (l_u8)(EE_CHIP_ID_1);
    data[3] = (l_u8)(EE_CHIP_ID_1 >> 8); /* MSB */
}
#endif /* (SL_HAS_SERIAL_NUMBER_CALLOUT == 1) */

/** Lin api event: application stop
 *
 * This function is called by the lin driver when it is initializing
 * to go into bootloader mode, this function should disable all critical
 * peripherals and modules.
 * @retval  true   command has been handled successful;
 * @retval  false  command has not been handled successful;
 */
bool fw_lepm_ApplicationStop(void)
{
    adc_Stop();
    pwm_Disable();
    swtimer_deinit();
    return (true); /* return that the application has stopped */
}

#if (SL_HAS_READ_BY_ID_CALLOUT == 1)
/** User defined read-by-identifier request.
 *
 * This callout is used when the master node transmits a read by identifier request with
 * an identifier in the user defined area. The slave node application will be called from
 * the driver when such request is received.
 * @param[in]   iii     ifc handler
 * @param[in]   id      identifier in the user defined area (32 to 63)
 * @param[in]   pci     pointer for setting PCI of response
 * @param[in]   data    pointer to response buffer
 * @return  Status:
 * - LD_NEGATIVE_RESPONSE   - the slave node will respond with a negative response;
 * - LD_POSTIVE_RESPONSE    - the slave node will setup a positive response;
 * - LD_NO_RESPONSE         - the slave node will not answer.
 */
l_u8 ld_read_by_id_callout(l_ifc_handle iii, l_u8 id, l_u8 *pci, l_u8 *data)
{
    l_u8 u8Return;
    (void)iii;
    switch (id)
    {
    case 0x21u:
    {
        uint8_t lin_cfg_len = SL_NUMBER_OF_DYNAMIC_MESSAGES + 1u;
        uint8_t lin_config[1 + SL_NUMBER_OF_DYNAMIC_MESSAGES] = SL_NODE_CONFIGURATION_INITIALIZER;

        /* Verify NAD */
#if (SL_HAS_SAVE_CONFIGURATION_SERVICE == 1)
        (void)eeprom_ReadLINconfig(lin_config, lin_cfg_len);

        *pci = 8u; /* 7-bytes of data + 1 */

        for (uint16_t index = 0u; index < lin_cfg_len && index < 7u; index++)
        {
            data[index] = lin_config[index];
        }
#endif
        u8Return = LD_POSITIVE_RESPONSE;
        break;
    }

    case 0x2Bu:
    {
        *pci = 5u; /* 4-bytes of data + 1-byte of pci */
        data[0] = (uint8_t)LIBRARIES_VERSION_MAJOR;
        data[1] = (uint8_t)LIBRARIES_VERSION_MINOR;
        data[2] = (uint8_t)LIBRARIES_VERSION_REVISION;
        data[3] = (uint8_t)LIBRARIES_VERSION_CUSTOMER_BUILD;
        u8Return = LD_POSITIVE_RESPONSE;
        break;
    }

    case 0x2Cu:
    {
        uint32_t version = VERSION_getFwAppVersion();
        *pci = 5u; /* 4-bytes of data + 1-byte of pci */
        data[0] = (uint8_t)(version >> 24);
        data[1] = (uint8_t)(version >> 16);
        data[2] = (uint8_t)(version >> 8);
        data[3] = (uint8_t)(version >> 0);
        u8Return = LD_POSITIVE_RESPONSE;
        break;
    }

    default:
        u8Return = LD_NEGATIVE_RESPONSE;
        break;
    }

    return (u8Return);
}
#endif /* (SL_HAS_READ_BY_ID_CALLOUT == 1) */

#if (SL_HAS_UNKNOWN_DIAG_CALLOUT == 1)
/** Application specific diagnostic frame handler
 *
 * @param[in]  transfer  Request and response data field.
 * @retval  true   Message was handled and response prepared.
 * @retval  false  Otherwise.
 */
bool ld_AppDiagRequest(LINDiagTransfer_t *transfer)
{
    bool retVal = false;

    switch (transfer->request.reqSId)
    {
#ifdef APP_HAS_DEBUG
    case 0xDB:
        retVal = DebugFrameHdlr(transfer->request.data,
                                (l_u8 *)&transfer->response.dataLen,
                                transfer->response.data);
        break;
#endif

    default:
        break;
    }

    if (retVal)
    {
        transfer->response.respSId = transfer->request.reqSId + 0x40;
    }

    return retVal;
}

#ifdef APP_HAS_DEBUG
/**
 * Debug frames - diagnostics SID=0xDB
 */
bool DebugFrameHdlr(l_u8 inData[5], l_u8 *pci, l_u8 data[])
{
    bool bReturn = false;
    *pci = 5u; /* 5-bytes of data */

    switch (inData[4])
    {
#if (DEBUG_DB_B2 == 1)
    case 0xB2: /* https://gitlab.melexis.com/bu-act/wiki/wikis/lin_debug/lin_debug_home#0xb2 */
    {
        switch (inData[3])
        {
        case 0:
        {
            /*
             * Set buffer address
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             *  | NAD | PCI |  SID |    D0    |    D1    |    D2    |    D3    |    D4    |
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             *  | NAD | 0x06| Debug| ADDRESS  | ADDRESS  | BUFFER   | SUBFUNC  |   FUNC   |
             *  |     |     | 0xDB | LSB      | MSB      | 0..2     | 0        |   0xB2   |
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             */
            uint16_t addr = (((uint16_t)inData[0]) + (((uint16_t)inData[1]) << 8));

            if (addr != 0u)
            {
                switch (inData[2])
                {
                case 0:
                    l_pu16DebugBufferAddress[0] = (uint16_t *)addr;
                    bReturn = true;
                    break;
                case 1:
                    l_pu16DebugBufferAddress[1] = (uint16_t *)addr;
                    bReturn = true;
                    break;
                case 2:
                    l_pu16DebugBufferAddress[2] = (uint16_t *)addr;
                    bReturn = true;
                    break;
                default:
                    break;
                }
            }
            break;
        }
        case 1:
            /*
             * Trigger
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             *  | NAD | PCI |  SID |    D0    |    D1    |    D2    |    D3    |    D4    |
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             *  | NAD | 0x06| Debug|          |          | BUFFERS  | SUBFUNC  |   FUNC   |
             *  |     |     | 0xDB |          |          | 1..3     | 1        |   0xB2   |
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             */
            l_u16DebugBufferElements = inData[2] & 0x03;
            l_u16DebugBufferSize = DEBUG_BUFFER_SIZE / l_u16DebugBufferElements;
            if (l_u16DebugBufferElements == 1)
            {
                g_u16DebugCaptureTrigger[0] = 0u;
                g_u16DebugCaptureTrigger[1] = DEBUG_BUFFER_SIZE; /* not used */
                g_u16DebugCaptureTrigger[2] = DEBUG_BUFFER_SIZE; /* not used */
                g_u16DebugCaptureEnd[0] = DEBUG_BUFFER_SIZE;
            }
            else if (l_u16DebugBufferElements == 2)
            {
                g_u16DebugCaptureTrigger[0] = 0u;
                g_u16DebugCaptureTrigger[1] = DEBUG_BUFFER_SIZE / 2;
                g_u16DebugCaptureTrigger[2] = DEBUG_BUFFER_SIZE; /* not used */
                g_u16DebugCaptureEnd[0] = DEBUG_BUFFER_SIZE / 2;
                g_u16DebugCaptureEnd[1] = DEBUG_BUFFER_SIZE;
            }
            else if (l_u16DebugBufferElements == 3)
            {
                g_u16DebugCaptureTrigger[0] = 0u;
                g_u16DebugCaptureTrigger[1] = DEBUG_BUFFER_SIZE / 3;
                g_u16DebugCaptureTrigger[2] = (2 * DEBUG_BUFFER_SIZE) / 3;
                g_u16DebugCaptureEnd[0] = DEBUG_BUFFER_SIZE / 3;
                g_u16DebugCaptureEnd[1] = (2 * DEBUG_BUFFER_SIZE) / 3;
                g_u16DebugCaptureEnd[2] = DEBUG_BUFFER_SIZE;
            }
            adc_RegisterIRQ2(B2_exit);
            bReturn = true;
            break;
        case 2:
            data[0] = (uint8_t)g_u16DebugCaptureTrigger[0];
            data[1] = (uint8_t)(g_u16DebugCaptureTrigger[0] >> 8);
            data[2] = (uint8_t)l_u16DebugBufferSize;
            data[3] = (uint8_t)(l_u16DebugBufferSize >> 8);
            bReturn = true;
            break;
        case 3:
        {
            /*
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             *  | NAD | PCI |  SID |    D0    |    D1    |    D2    |    D3    |    D4    |
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             *  | NAD | 0x06| Debug| INDEX    | INDEX    | BUFFER   | SUBFUNC  |   FUNC   |
             *  |     |     | 0xDB | LSB      | MSB      |          | 3        |   0xB2   |
             *  +-----+-----+------+----------+----------+----------+----------+----------+
             */
            uint16_t index = ((uint16_t)inData[0]) + (((uint16_t)inData[1]) << 8);
            index += (inData[2] * l_u16DebugBufferSize);
            uint16_t temp;
            temp = (uint16_t)(l_u16DebugBuffer[index]);
            data[0] = (uint8_t)temp;
            data[1] = (uint8_t)(temp >> 8);
            temp = (uint16_t)(l_u16DebugBuffer[index + 1]);
            data[2] = (uint8_t)temp;
            data[3] = (uint8_t)(temp >> 8);
            temp = (uint16_t)(l_u16DebugBuffer[index + 2]);
            data[4] = (uint8_t)temp;
            data[5] = (uint8_t)(temp >> 8);
            temp = (uint16_t)(l_u16DebugBuffer[index + 3]);
            data[6] = (uint8_t)temp;
            data[7] = (uint8_t)(temp >> 8);
            temp = (uint16_t)(l_u16DebugBuffer[index + 4]);
            data[8] = (uint8_t)temp;
            data[9] = (uint8_t)(temp >> 8);
            temp = (uint16_t)(l_u16DebugBuffer[index + 5]);
            data[10] = (uint8_t)temp;
            data[11] = (uint8_t)(temp >> 8);
            data[12] = 0u;
            *pci = 13u; /* 13-bytes of data */
            bReturn = true;
            break;
        }
        case 4:
            /* stop */
            g_u16DebugCaptureTrigger[0] = DEBUG_BUFFER_SIZE;
            g_u16DebugCaptureTrigger[1] = DEBUG_BUFFER_SIZE;
            g_u16DebugCaptureTrigger[2] = DEBUG_BUFFER_SIZE;
            adc_RegisterIRQ2(NULL);
            bReturn = true;
            break;
        default:
            bReturn = false;
            break;
        }
        break;
    }
#endif /* (DEBUG_DB_B2 == 1) */
#if (DEBUG_DB_B3 == 1)
    case 0xB3: /* https://gitlab.melexis.com/bu-act/wiki/wikis/lin_debug/lin_debug_home#0xb3 */
    {
        /* Operate on target
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         *  | NAD | PCI |  SID |    D0    |    D1    |    D2    |    D3    |    D4    |
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         *  | NAD | 0x06| Debug| Addr     | Addr     | Value    | Value    |   FUNC   |
         *  |     |     | 0xDB | LSB      | MSB      | LSB      | MSB      |   0xB3   |
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         */
        uint16_t addr = (uint16_t)((uint16_t)inData[0] + (((uint16_t)inData[1]) << 8));

        if ((addr >= MEM_RAM_START) && (addr <= MEM_RAM_START + MEM_RAM_SIZE)) /* RAM address range */
        {
            bReturn = true;

            if (0 != (addr & 0x8000)) /* we have a byte write request */
            {
                uint8_t *const bytePtr = (uint8_t *)(addr & 0x7FFF);
                *bytePtr = (uint8_t)inData[2];
            }
            else
            {
                if (1 == (addr & 0x0001))
                {
                    /* can't write word on odd address */
                    bReturn = false;
                }
                else
                {
                    uint16_t tmp = (uint16_t)inData[2] + (((uint16_t)inData[3]) << 8U);
                    uint16_t *const wordPtr = (uint16_t *)addr;
                    *wordPtr = tmp;
                }
            }

            data[0] = 0;
            data[1] = 0;
            data[2] = 0;
            data[3] = 0;
            data[4] = 0;
        }

        break;
    }
#endif /* DEBUG_DB_B3 == 1 */
#if (DEBUG_DB_B4 == 1)
    case 0xB4:
    {
        /* Write to eeprom (12.5ms!)
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         *  | NAD | PCI |  SID |    D0    |    D1    |    D2    |    D3    |    D4    |
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         *  | NAD | 0x06| Debug| Addr     | Addr     | Data     | Data     |   FUNC   |
         *  |     |     | 0xDB | LSB      | MSB      | LSB      | MSB      |   0xB4   |
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         */
        const uint16_t addr = (uint16_t)((uint16_t)inData[0]) + (((uint16_t)inData[1]) << 8U);

        if ((addr >= MEM_EEPROM_START) && (addr < MEM_EEPROM_START + MEM_EEPROM_SIZE))
        {
            uint16_t data64bit[4];
            data64bit[0] = (uint16_t)((uint16_t)inData[2] + (((uint16_t)inData[3]) << 8));
            data64bit[1] = (uint16_t)((uint16_t)inData[5] + (((uint16_t)inData[6]) << 8));
            data64bit[2] = (uint16_t)((uint16_t)inData[7] + (((uint16_t)inData[8]) << 8));
            data64bit[3] = (uint16_t)((uint16_t)inData[9] + (((uint16_t)inData[10]) << 8));

            if (!EEPROM_getEEBUSY())
            {
                ENTER_SECTION(ATOMIC_SYSTEM_MODE);
                EEPROM_WriteWord64_blocking(addr, &data64bit[0], 0x07u);
                EXIT_SECTION();
                *pci = 0u; /* 0-bytes of data */
                bReturn = true;
            }
        }
        break;
    }
#endif /* (DEBUG_DB_B4 == 1) */
#if (DEBUG_DB_B5 == 1)
    case 0xB5:
    {
        const uint16_t addr = (uint16_t)((uint16_t)inData[0]) + (((uint16_t)inData[1]) << 8U);

        if ((addr >= MEM_EEPROM_START) && (addr < MEM_EEPROM_START + MEM_EEPROM_SIZE))
        {
            EEPROM_ClearErrorFlags();
            (void)memcpy((void *)data, (void *)addr, 8u);
            if (EEPROM_GetErrorFlags() ? false : true)
            {
                *pci = 8; /* 8-bytes of data */
                bReturn = true;
            }
        }
        break;
    }
#endif
#if (DEBUG_DB_B8 == 1)
    case 0xB8: /* https://gitlab.melexis.com/bu-act/wiki/wikis/lin_debug/lin_debug_home#0xb8 */
    {
        /* Raw peek
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         *  | NAD | PCI |  SID |    D0    |    D1    |    D2    |    D3    |    D4    |
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         *  | NAD | 0x06| Debug| Addr     | Addr     | len      | Addr     |   FUNC   |
         *  |     |     | 0xDB | LSB      | MSB      |          | Ext      |   0xB8   |
         *  +-----+-----+------+----------+----------+----------+----------+----------+
         */
        uint16_t len = (uint16_t)inData[2];
#ifdef HAS_MEM_GT_64KB
        if (inData[3] == 1U)
        {
            /* read beyond 64K, currently up to 128K, extend when needed */
            uint32_t addr = (uint32_t)((uint16_t)inData[0] + ((uint16_t)inData[1] << 8U)) + 0x10000UL;

            if ((addr != 0) && (len != 0) && (len <= (LDT_MAX_DATA_IN_SEGMENTED_TRANSFER - 2)))
            {
                for (uint16_t index = 0u; index < len; index++)
                {
                    data[index] = memoryReadByte(addr);
                    addr++;
                }
            }
        }
        else
#endif /* HAS_MEM_GT_64KB */
        {
            const uint8_t *addr = (uint8_t *)((uint16_t)inData[0]) + (((uint16_t)inData[1]) << 8U);

            if ((addr != NULL) && (len != 0) && (len <= (LDT_MAX_DATA_IN_SEGMENTED_TRANSFER - 2)))
            {
                for (uint16_t index = 0u; index < len; index++)
                {
                    data[index] = addr[index];
                }
                *pci = len; /* x-bytes of data */
                bReturn = true;
            }
        }
        break;
    }
#endif /* (DEBUG_DB_B8 == 1) */
    default:
        bReturn = false;
        break;
    }
    return (bReturn);
}
#endif /* APP_HAS_DEBUG */

#endif /* (SL_HAS_UNKNOWN_DIAG_CALLOUT == 1) */

/** Sleep mode notification handler
 *
 * @param[in]  Reason  Reason of entering sleep mode.
 */
void l_ifc_sleep_entered_callout(ml_SleepReason_t Reason)
{
    switch (Reason)
    {
    case ml_reasonMaster:
        /* "Go-to-Sleep" frame has been received from Master */
        EVENT_GotoSleep(); // 250415 - To do act after sr
        lin22_GotoSleep(); /* switch to sleep mode */
        break;
    case ml_reasonTimeOut:
        /* Sleep State entered with a timeout with LIN bus recessive */
        bLinTimeoutActive = 1;
#if LIN_TIMEOUT_ENABLE == 0
        break;
#endif
    case ml_reasonTimeOutDominant:
    /* Sleep State entered with a timeout with LIN bus dominant */
    case ml_reasonWakeupAbort:
    /* LIN cluster is still inactive after 3 wake-up pulses */
    case ml_reasonCommand:
    /* Sleep State entered with an application command */
    default:
#if LIN_SLEEP_ENABLE == 1
        /*
         * 2. Real application
         * The following commands have to be enabled in a real application to
         * switch the device into sleep mode after 4s of LIN bus inactivity.
         */
        EVENT_GotoSleep();
        lin22_GotoSleep(); /* switch to sleep mode */
#endif
        break;
    }
}

#ifdef APP_HAS_DEBUG
#if (DEBUG_DB_B2 == 1)
void B2_exit(void)
{
    if (g_u16DebugCaptureCounter >= g_u16DebugCaptureDivider)
    {
        g_u16DebugCaptureCounter = 0;
        if (g_u16DebugCaptureTrigger[0] < g_u16DebugCaptureEnd[0])
        {
            l_u16DebugBuffer[g_u16DebugCaptureTrigger[0]++] = (uint16_t)(*l_pu16DebugBufferAddress[0]);
        }
        if (g_u16DebugCaptureTrigger[1] < g_u16DebugCaptureEnd[1])
        {
            l_u16DebugBuffer[g_u16DebugCaptureTrigger[1]++] = (uint16_t)(*l_pu16DebugBufferAddress[1]);
        }
        if (g_u16DebugCaptureTrigger[2] < g_u16DebugCaptureEnd[2])
        {
            l_u16DebugBuffer[g_u16DebugCaptureTrigger[2]++] = (uint16_t)(*l_pu16DebugBufferAddress[2]);
        }
    }
    else
    {
        g_u16DebugCaptureCounter++;
    }
}
#endif /* DEBUG_DB_B2 */
#endif /* APP_HAS_DEBUG */

/* EOF */
