/**
 ******************************************************************************
 * @file    lin_api.h
 * @brief   LIN API
 ******************************************************************************
 * @copyright (C) 2016-2018 Melexis N.V. All rights reserved.
 * @internal
 * Melexis N.V. is supplying this software for use with Melexis N.V.
 * processor based microcontrollers only.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * MELEXIS N.V. SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 * @endinternal
 *
 * @details The LIN API is a network software layer that hides the details of a LIN network
 *          configuration (e.g. how signals are mapped into certain frames) for a user making an application
 *          program for an arbitrary ECU. Instead the user will be provided an API, which is focused on
 *          the signals transported on the LIN network.
 *
 *          The LIN API consist of two parts:
 *          -# Static LIN API
 *          -# LDF-dependent LIN API (lin_signals.h)
 *
 *          The Static LIN API is divided on tree groups:
 *          -# LIN Common API (lin_api.h)
 *          -# LIN Slave specific API (lin_api_sl.h)
 *          -# LIN Master specific API (lin_api_ma.h)
 *
 *          The LDF-dependent LIN API (lin_signals.h) is common for all used LIN interfaces.
 *
 *
 * @ingroup lin_api
 *
 */

#ifndef LIN_API_H
#define LIN_API_H

#include <stdint.h>
#include <stdbool.h>


/* Platform/device specific definitions */
#include "lin_api_config.h"

/* Include LDF specific API */
#include "lin_signals.h"


/*-----------------------------------------------------------------------------
 * Macros
 */

/*
 * Versions
 */

/* STD LIN API version 2.x.x */
#define STD_LIN_API_VERSION_MAJOR   3
#define STD_LIN_API_VERSION_MINOR   0
#define STD_LIN_API_VERSION_PATCH   0
#define STD_LIN_API_VERSION_WIP     0 /* 0: released; 1: work in process */

/** Helper macro for versions */
#define LIN_1_3  0
#define LIN_2_0  1
#define LIN_2_1  2
#define LIN_2_2  3
#define SAE_J2602_2012  4
#define ISO_17987_2016  5

/* The ifc can be SL or MA in the following macro */
#define vLIN_1_3(ifc) ((ifc ## _API_VERSION == LIN_1_3))
#define vLIN_2_0(ifc) ((ifc ## _API_VERSION == LIN_2_0))
#define vLIN_2_1(ifc) ((ifc ## _API_VERSION == LIN_2_1))
#define vLIN_2_2(ifc) ((ifc ## _API_VERSION == LIN_2_2))
#define vSAE_J2602_2012(ifc) ((ifc ## _API_VERSION == SAE_J2602_2012))
#define vISO_17987_2016(ifc) ((ifc ## _API_VERSION == ISO_17987_2016))

#define vLIN(ifc) (vLIN_1_3(ifc) || vLIN_2_0(ifc) || vLIN_2_1(ifc) || vLIN_2_2(ifc))
#define vLIN_2_x(ifc) (vLIN_2_0(ifc) || vLIN_2_1(ifc) || vLIN_2_2(ifc))
#define vLIN_2_1_plus(ifc) (vLIN_2_1(ifc) || vLIN_2_2(ifc))

/** Helper macro for configurations */
#define IS_ENABLED_CONF(conf) ((conf) == 1)
#define IS_DISABLED_CONF(conf) ((conf) == 0)

/** Wrapper to get atomic code execution block for STD LIN API */
#define ENTER_STD_LIN_API_ATOMIC_SECTION()  \
    {                                   \
        l_irqmask m_reg_saved __attribute__((cleanup(l_sys_irq_restore_pointer))); \
        m_reg_saved = l_sys_irq_disable();

#define EXIT_STD_LIN_API_ATOMIC_SECTION()   \
    }


/*-----------------------------------------------------------------------------
 * Common Types definition
 */

/**
 * LIN Standard types
 */

/** @name LIN Standard API's types */
typedef bool l_bool;
typedef uint16_t l_irqmask;
typedef uint8_t l_u8;
typedef uint16_t l_u16;
/**@}*/

/** LIN interface determination */
typedef enum {
    ifcSlave_1 = 1,   /**< Determine LIN Slave interface */
    ifcMaster_1       /**< Determine LIN Master interface */
} l_ifc_handle;

#if (LIN_MASTER_API == 1)
/** Schedule table commands */
typedef enum {

    sc_Frame = 0                /**< Frame processing */

/* start MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
    ,sc_MasterReq               /**< Master request sending */
    ,sc_SlaveResp               /**< Slave response receiving */
    ,sc_AssignNAD               /**< Generates an assign NAD request */
    ,sc_FreeFormat              /**< Transmits a fixed master request frame with the eight data bytes provided */

/* start MA_vLIN_2_0 || MA_vSAE_J2602_2012 */
    ,sc_AssignFrameId           /**< Generates an Assign_frame_id request */
    ,sc_UnassignFrameId         /**< Generates an Unassign_frame_id request */
/* end MA_vLIN_2_0 || MA_vSAE_J2602_2012 */

/* start MA_vLIN_2_1_plus || MA_vISO17987_2016 */
    ,sc_ConditionalChangeNAD    /**< Generates a conditional change NAD request */
    ,sc_DataDump                /**< Generates a data dump request */
    ,sc_SaveConfiguration       /**< Generates a save configuration request */
    ,sc_AssignFrameIdRange      /**< Generates an assign frame PID range request */
/* end MA_vLIN_2_1_plus || MA_vISO17987_2016 */

    /* Note: For internal use only, can't be scheduled */
    ,sc_GoToSleep               /**< Interleaved GoToSleep command was received from the application */
/* end MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

} l_ScheduleCommand_t;

/** Schedule table instance type */
typedef struct {
    l_ScheduleCommand_t ScheduleCommand;    /**< Scheduled command type */
    void *FunctionArgs;                     /**< Scheduled command arguments l_m_Frame_t, l_m_AssignNAD_t,
                                                 l_m_CondChangeNAD_t, l_m_DataDump_t, l_m_SaveConfig_t,
                                                 l_m_AssignFrameIdRange_t, l_m_FreeFormat_t, l_m_AssignFrameId_t,
                                                 l_m_UnassignFrameId_t.
                                                 NOTE: The FunctionArgs can be unspecified for some command types */
    l_u8 Delay;                             /**< Delay [time_base_ticks] */
} l_ScheduleInstance_t;

/** Schedule table type */
typedef struct {
    l_ScheduleInstance_t *ScheduleInstances;    /**< Schedule entity */
    l_u16 ScheduleSize;                         /**< Schedule table size */
} l_Schedule_t;

/** @name LIN Standard API's types */
typedef l_Schedule_t *l_schedule_handle;
/**@}*/
#endif /* (LIN_MASTER_API == 1) */


/*-----------------------------------------------------------------------------
 * Interface and LDF specific functionality including
 */

/** @name User provided call-outs */
extern l_irqmask l_sys_irq_disable (void);
extern void l_sys_irq_restore (l_irqmask previous);
/**@}*/


/*-----------------------------------------------------------------------------
 * Helper function to restore the IRQ
 */

STATIC INLINE void l_sys_irq_restore_pointer (const l_irqmask *p);
#ifndef UNITTEST
#include "lin_api_inline_impl.h"
#endif /* UNITTEST */

/* All needed definition from lin_api.h are provided */
#define LIN_API_GENERAL_DEFS
#if (LIN_SLAVE_API == 1) || (LIN_MASTER_API == 1)
/* Include LDF specific API */
#include "lin_signals.h"


/*-----------------------------------------------------------------------------
 *  Externals
 */
extern const l_ifc_handle dummy;


/*-----------------------------------------------------------------------------
 * Common function prototypes
 */

/** @name Driver and cluster management */
extern l_bool l_sys_init(void);
/**@}*/

/* Isn't currently supported */
#if 0
/* Transport layer */
/** @name Transport layer */
extern void ld_init (l_ifc_handle iii);
/**@}*/

/** @name Transport layer - Raw API */
extern void ld_put_raw (l_ifc_handle iii, const l_u8* const data);
extern void ld_get_raw (l_ifc_handle iii, l_u8* const data);
extern l_u8 ld_raw_tx_status (l_ifc_handle iii);
extern l_u8 ld_raw_rx_status (l_ifc_handle iii);
/**@}*/

/** @name Transport layer - Cooked API */
extern void ld_send_message (l_ifc_handle iii, l_u16 length, l_u8 NAD, const l_u8* const data);
extern void ld_receive_message (l_ifc_handle iii, l_u16* const length, l_u8* const NAD, l_u8* const data);
extern l_u8 ld_tx_status (l_ifc_handle iii);
extern l_u8 ld_rx_status (l_ifc_handle iii);
/**@}*/
#endif

#endif /* (LIN_SLAVE_API == 1) || (LIN_MASTER_API == 1) */

#if LIN_MASTER_API == 1
/* Include LIN Master specific API */
#include "lin_api_ma.h"
#endif /* LIN_MASTER_API */

#if LIN_SLAVE_API == 1
/* Include LIN Slave specific API */
#include "lin_api_sl.h"
#endif /* LIN_SLAVE_API */

#if LIN_MASTER_API != 1
/* Enable compatibility mode for the application */
#include "lin_api_compat.h"
#endif /* LIN_MASTER_API */

#endif /* LIN_API_H */
/* EOF */
