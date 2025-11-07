/**
 ******************************************************************************
 * @file    lin_api_ma.h
 * @brief   LIN Master API
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
 *          The API is split in three sections:
 *          -# LIN core API (lin_core_ma.h)
 *          -# LIN node configuration and identification API (lin_cfg_ma.h)
 *          -# LIN transport layer API (optional) (lin_trans_ma.h)
 *
 * @note The implementation relies on LIN 2.2 specifications
 *
 * @ingroup lin_master_api
 *
 */

#ifndef LIN_API_MA_H
#define LIN_API_MA_H


#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif


//TODO: Add aliases for dynamic calls

/*-----------------------------------------------------------------------------
 * Macros
 */

/* Helper macro for configurations */

#define MA_EN_EVENT_TRIGGERED_FRAME             IS_ENABLED_CONF(MA_HAS_EVENT_TRIGGERED_FRAME)
#define MA_EN_SPORADIC_FRAME                    IS_ENABLED_CONF(MA_HAS_SPORADIC_FRAME)
#define MA_EN_MASTER_REQ_COMMAND                IS_ENABLED_CONF(MA_HAS_MASTER_REQ_COMMAND)
#define MA_EN_SLAVE_RESP_COMMAND                IS_ENABLED_CONF(MA_HAS_SLAVE_RESP_COMMAND)
#define MA_EN_ASSIGN_NAD_COMMAND                IS_ENABLED_CONF(MA_HAS_ASSIGN_NAD_COMMAND)
#define MA_EN_FREE_FORMAT_COMMAND               IS_ENABLED_CONF(MA_HAS_FREE_FORMAT_COMMAND)
#define MA_EN_CONDITIONAL_CHANGE_NAD_COMMAND    IS_ENABLED_CONF(MA_HAS_CONDITIONAL_CHANGE_NAD_COMMAND)
#define MA_EN_DATA_DUMP_COMMAND                 IS_ENABLED_CONF(MA_HAS_DATA_DUMP_COMMAND)
#define MA_EN_SAVE_CONFIGURATION_COMMAND        IS_ENABLED_CONF(MA_HAS_SAVE_CONFIGURATION_COMMAND)
#define MA_EN_ASSIGN_FRAME_ID_RANGE_COMMAND     IS_ENABLED_CONF(MA_HAS_ASSIGN_FRAME_ID_RANGE_COMMAND)
#define MA_EN_ASSIGN_FRAME_ID_COMMAND           IS_ENABLED_CONF(MA_HAS_ASSIGN_FRAME_ID_COMMAND)
#define MA_EN_UNASSIGN_FRAME_ID_COMMAND         IS_ENABLED_CONF(MA_HAS_UNASSIGN_FRAME_ID_COMMAND)

#define MA_DIS_EVENT_TRIGGERED_FRAME            IS_ENABLED_CONF(MA_HAS_EVENT_TRIGGERED_FRAME)
#define MA_DIS_SPORADIC_FRAME                   IS_ENABLED_CONF(MA_HAS_SPORADIC_FRAME)
#define MA_DIS_MASTER_REQ_COMMAND               IS_DISABLED_CONF(MA_HAS_MASTER_REQ_COMMAND)
#define MA_DIS_SLAVE_RESP_COMMAND               IS_DISABLED_CONF(MA_HAS_SLAVE_RESP_COMMAND)
#define MA_DIS_ASSIGN_NAD_COMMAND               IS_DISABLED_CONF(MA_HAS_ASSIGN_NAD_COMMAND)
#define MA_DIS_FREE_FORMAT_COMMAND              IS_DISABLED_CONF(MA_HAS_FREE_FORMAT_COMMAND)
#define MA_DIS_CONDITIONAL_CHANGE_NAD_COMMAND   IS_DISABLED_CONF(MA_HAS_CONDITIONAL_CHANGE_NAD_COMMAND)
#define MA_DIS_DATA_DUMP_COMMAND                IS_DISABLED_CONF(MA_HAS_DATA_DUMP_COMMAND)
#define MA_DIS_SAVE_CONFIGURATION_COMMAND       IS_DISABLED_CONF(MA_HAS_SAVE_CONFIGURATION_COMMAND)
#define MA_DIS_ASSIGN_FRAME_ID_RANGE_COMMAND    IS_DISABLED_CONF(MA_HAS_ASSIGN_FRAME_ID_RANGE_COMMAND)
#define MA_DIS_ASSIGN_FRAME_ID_COMMAND          IS_DISABLED_CONF(MA_HAS_ASSIGN_FRAME_ID_COMMAND)
#define MA_DIS_UNASSIGN_FRAME_ID_COMMAND        IS_DISABLED_CONF(MA_HAS_UNASSIGN_FRAME_ID_COMMAND)

/* Check for the case when Master request command is disabled while other commands are used */
#if (MA_vLIN_2_0 || MA_vSAE_J2602_2012) && MA_DIS_MASTER_REQ_COMMAND && \
    (MA_EN_SLAVE_RESP_COMMAND || \
     MA_EN_ASSIGN_NAD_COMMAND || \
     MA_EN_FREE_FORMAT_COMMAND || \
     ((MA_vLIN_2_1_plus || MA_vISO17987_2016) && \
    (MA_EN_CONDITIONAL_CHANGE_NAD_COMMAND || \
     MA_EN_DATA_DUMP_COMMAND || \
     MA_EN_SAVE_CONFIGURATION_COMMAND || \
     MA_EN_ASSIGN_FRAME_ID_RANGE_COMMAND)) || \
    MA_EN_ASSIGN_FRAME_ID_COMMAND || \
    MA_EN_UNASSIGN_FRAME_ID_COMMAND)
#error "Wrong configuration: the Master request command need to be enabled"
#endif /* MA_DIS_MASTER_REQ_COMMAND with enabled other commands */


/*-----------------------------------------------------------------------------
 * Function prototypes
 */

/** @name Interface management */
#if MA_vLIN_2_1_plus || MA_vISO17987_2016
extern l_bool l_ifc_init_ma1 (void);
#else /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */
extern void l_ifc_init_ma1 (void);
extern l_bool l_ifc_connect_ma1 (void);
extern l_bool l_ifc_disconnect_ma1 (void);
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
extern void l_ifc_goto_sleep_ma1 (void);
extern void l_ifc_wake_up_ma1 (void);
extern l_u16 l_ifc_read_status_ma1 (void);
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

/** @name Schedule management */
extern l_u8 l_sch_tick_ma1 (void);
extern void l_sch_set_ma1 (l_schedule_handle schedule, l_u8 entry);
/**@}*/

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
/** @name Node configuration */
extern void ld_assign_NAD (l_ifc_handle iii, l_u8 initial_NAD, l_u16 supplier_id, l_u16 function_id, l_u8 new_NAD);
extern void ld_conditional_change_NAD (l_ifc_handle iii,
                                       l_u8 NAD,
                                       l_u8 id,
                                       l_u8 byte,
                                       l_u8 mask,
                                       l_u8 invert,
                                       l_u8 new_NAD);

#if MA_vLIN_2_1_plus || MA_vISO17987_2016
extern l_u8 ld_is_ready (l_ifc_handle iii);
extern void ld_check_response (l_ifc_handle iii, l_u8* const RSID, l_u8* const error_code);

extern void ld_assign_frame_id_range (l_ifc_handle iii, l_u8 NAD, l_u8 start_index, const l_u8* const PIDs);
extern void ld_save_configuration (l_ifc_handle iii, l_u8 NAD);
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */

#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
extern l_bool ld_is_ready (l_ifc_handle iii);
extern l_u8 ld_check_response (l_ifc_handle iii, l_u8* RSID, l_u8* error_code);
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */

/* The Master node with version >= 2.0 can support Slave nodes v2.0.
 * The functionality options shall not depend from the Master node version */
#if MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND
void ld_assign_frame_id (l_ifc_handle iii, l_u8 NAD, l_u16 supplier_id, l_u16 message_id, l_u8 PID);
#endif /* MA_EN_ASSIGN_FRAME_ID_COMMAND || MA_EN_UNASSIGN_FRAME_ID_COMMAND */
/**@}*/

/** @name Identification */
extern void ld_read_by_id (l_ifc_handle iii, l_u8 NAD, l_u16 supplier_id, l_u16 function_id, l_u8 id, l_u8* const data);
/**@}*/


/*-----------------------------------------------------------------------------
 * Externals
 */

extern const l_Schedule_t L_NULL_SCHEDULE;


#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

#endif /* LIN_API_MA_H */


/* EOF */
