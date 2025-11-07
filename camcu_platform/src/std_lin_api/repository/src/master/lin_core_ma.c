/**
 ******************************************************************************
 * @file    lin_core_ma.c
 * @brief   LIN Master core API
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
 * @details The LIN core API handles initialization, processing and a signal based interaction
 *          between the application and the LIN core.
 *
 * @ingroup lin_master_api
 *
 * @defgroup lin_master_api LIN Master API
 * @{
 * @brief   LIN Master API
 * @details The LIN API is a network software layer that hides the details of a LIN network
 *          configuration (e.g. how signals are mapped into certain frames) for a user making an application
 *          program for an arbitrary ECU. Instead the user will be provided an API, which is focused on
 *          the signals transported on the LIN network. See lin_api_ma.h for information about the interface.
 * @}
 *
 */

/* Common API */
#include "lin_api.h"

#if LIN_MASTER_API == 1

#include "lin_core.h"

/* Interface specific API */
#include "lin_core_ma.h"
#include "lin_trans_ma.h"
#include "lin_cfg_ma.h"


/*-----------------------------------------------------------------------------
 * Macros
 */

#if MA_LIN_BAUDRATE == 19200
    #define MA_LIN_BAUDRATE_MODE    0
#elif MA_LIN_BAUDRATE == 10417
    #define MA_LIN_BAUDRATE_MODE    1
#elif MA_LIN_BAUDRATE == 9600
    #define MA_LIN_BAUDRATE_MODE    2
#else
    #define MA_LIN_BAUDRATE_MODE    0
    #warning "The incorrect 'MA_LIN_BAUDRATE' value is defined; Default 19200 baudrate is used by LIN Master"
#endif


/*-----------------------------------------------------------------------------
 * Types definition
 */

/** Schedule handler type */
typedef struct {
    l_Schedule_t *Schedule;     /* Schedule table pointer */
    l_u8 CurrentEntryPoint;     /* Current entry point */
} l_ScheduleHandler_t;

/* Schedule table processing details type */
typedef struct {
    l_ScheduleHandler_t ActiveSchedule;         /**< Currently active schedule table */
    l_ScheduleHandler_t InterruptedSchedule;    /**< Interrupted schedule table for event-triggered collision resolve */
    l_ScheduleHandler_t RequestedSchedule;      /**< Newly requested schedule table */
    l_bool isEventTrigCollision;  /**< True when event-triggered collision resolving schedule table was activated */
    l_bool isReqScheduleTable;    /**< True when switching to the new schedule table was requested */
    l_ScheduleInstance_t ActiveScheduleInstance;    /**< Schedule instance in processing */
    l_m_FrameHandler_t ActiveFrameHandler;          /**< Pointer to the Frame handler currently in processing */
} l_m_ScheduleInfo_t;


/*-----------------------------------------------------------------------------
 * Function prototypes
 */

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
static void l_m_ProcessGoToSleep (l_m_CmdProcEvent_t event);
static void l_m_UpdateIfcStatus (void);
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

static void l_m_IfcInitHelper (void);
static void l_m_ProcessScheduleCommand (l_m_CmdProcEvent_t event);
static void l_m_ProcessFrame (l_m_CmdProcEvent_t event);


/*-----------------------------------------------------------------------------
 * Variables
 */

/* Globals */
l_m_Driver_t l_m_Driver;

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
/* Status word */
l_m_IfcStatusHelper_t l_m_IfcStatusHelper;
static l_m_IfcStatus_t l_m_IfcStatus; /* Interface status (def = 0x0000) */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

/* Schedule table processing details */
static l_m_ScheduleInfo_t l_m_ScheduleInfo;

/* Empty schedule table declaration */
static const l_ScheduleInstance_t NULL_ScheduleInstance[0];
const l_Schedule_t L_NULL_SCHEDULE = {(l_ScheduleInstance_t*)NULL_ScheduleInstance, 0};


/*-----------------------------------------------------------------------------
 * Functions
 */


/** The time-base tick processing.
 * The l_sch_tick function follows a schedule. When a frame becomes due, its transmission is initiated.
 * When the end of the current schedule is reached, l_sch_tick starts again at the beginning of the schedule.
 * @note The LIN Master API behaviour is undefined in the case the new frame processing is requested by this function before the end of previous one.
 *       It's the responsibility of the cluster design to guarantee long enough frame slot to handle the frames.
 * @return next call action[l_u8]:
 * - Non-zero. If the next call of l_sch_tick will start the transmission of the frame in the next
 * schedule table entry. The return value will in this case be the next schedule table entry's number
 * (counted from the beginning of the schedule table) in the schedule table. The return value will be
 * in range 1 to N if the schedule table has N entries;
 * - Zero. If the next call of l_sch_tick will not start transmission of a frame.
 */
l_u8 l_sch_tick_ma1 (void)
{
    /* Note: Signals are updated independently from l_sch_tick_ma1 after the Frame was successfully transmitted / received */

    l_u8 retVal = 0u;

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
    /* Update IFC status */
    l_m_UpdateIfcStatus();
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

    /* If normal schedule table is running */
    if (l_m_ScheduleInfo.ActiveSchedule.Schedule->ScheduleSize != 0u) {
        /* Follow schedule table */
        l_m_ScheduleInfo.ActiveScheduleInstance.Delay--;    /* One more tick has been passed */
    } else {
        /* Set always 0 for L_NULL_SCHEDULE */
        l_m_ScheduleInfo.ActiveScheduleInstance.Delay = 0;
    }

    /*
     * Process schedule command
     */
    if (l_m_ScheduleInfo.ActiveScheduleInstance.Delay == 0u) {

        /*
         * It's time to process the schedule command
         */

        /* Switch to the new schedule table if it was requested */

        ENTER_STD_LIN_API_ATOMIC_SECTION();
        if (l_m_ScheduleInfo.isReqScheduleTable == true) {
            l_m_ScheduleInfo.ActiveSchedule = l_m_ScheduleInfo.RequestedSchedule;
            l_m_ScheduleInfo.isReqScheduleTable = false;        /* Clear request flag */
        }
        EXIT_STD_LIN_API_ATOMIC_SECTION();

        /* If normal schedule table is running (not L_NULL_SCHEDULE) */
        if (l_m_ScheduleInfo.ActiveSchedule.Schedule->ScheduleSize != 0u) {

            /*
             * Start of Frame Slot
             */

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
            /*
             * If Go to Sleep command was requested
             */
            if (l_m_IfcStatusHelper.isGotoSleep == true) {
                /* Schedule the GoToSleep command instead */
                l_m_ScheduleInfo.ActiveScheduleInstance.ScheduleCommand = sc_GoToSleep;

                /* As the current scheduled command has been interrupted by GoToSleep command,
                 * the Delay need to be reused from the interrupted schedule instance */
                l_u8 entryPointIdx = l_m_ScheduleInfo.ActiveSchedule.CurrentEntryPoint;
                l_m_ScheduleInfo.ActiveScheduleInstance.Delay =
                    l_m_ScheduleInfo.ActiveSchedule.Schedule->ScheduleInstances[entryPointIdx].Delay;

                /* Process the GoToSleep command */
                l_m_ProcessScheduleCommand(cpe_FrameSlotStart);

                ENTER_STD_LIN_API_ATOMIC_SECTION();
                l_m_IfcStatusHelper.FramesCnt++;     /* one more frame has been processed */
                EXIT_STD_LIN_API_ATOMIC_SECTION();
            } else {
#else /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
            if (true) {
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                /*
                 * Follow scheduled command
                 */

                /* Process current command */
                l_u8 entryPointIdx = l_m_ScheduleInfo.ActiveSchedule.CurrentEntryPoint;
                l_m_ScheduleInfo.ActiveScheduleInstance =
                    l_m_ScheduleInfo.ActiveSchedule.Schedule->ScheduleInstances[entryPointIdx];

                l_m_ProcessScheduleCommand(cpe_FrameSlotStart);
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                ENTER_STD_LIN_API_ATOMIC_SECTION();
                l_m_IfcStatusHelper.FramesCnt++;     /* one more frame has been processed */
                EXIT_STD_LIN_API_ATOMIC_SECTION();
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

                /* Set next entry point, new pending delay */
                l_m_ScheduleInfo.ActiveSchedule.CurrentEntryPoint++;
                if (l_m_ScheduleInfo.ActiveSchedule.CurrentEntryPoint >=
                    l_m_ScheduleInfo.ActiveSchedule.Schedule->ScheduleSize) {

                    /* TODO: Switch to diagnostic schedule table here if requested */

                    /* Switch back to interrupted schedule table when Event-Triggered collision resolving schedule table was completed */
                    if (l_m_ScheduleInfo.isEventTrigCollision == true) {
                        /* Set the interrupted schedule table */
                        l_m_ScheduleInfo.ActiveSchedule = l_m_ScheduleInfo.InterruptedSchedule;
                        l_m_ScheduleInfo.isEventTrigCollision = false;
                    } else {   /* Repeat current schedule table from the beginning */
                        l_m_ScheduleInfo.ActiveSchedule.CurrentEntryPoint = 0;
                    }
                } else {
                    /*
                     * The end of current schedule table isn't achieved yet
                     */
                }
            }
        } else {
            /*
             * Do nothings for L_NULL_SCHEDULE
             */
        }
    } else if (l_m_ScheduleInfo.ActiveScheduleInstance.Delay == 1u) {       /* Return next schedule entry; Wait the next tick */
        /*
         * Notify about next schedule command
         */
        retVal = l_m_ScheduleInfo.ActiveSchedule.CurrentEntryPoint + 1u;    /* Indexing is started from one */
    } else {
        /*
         * Next call of l_sch_tick will not start transmission of a frame. Return zero
         */
    }

    return retVal;
}


/** Set up the next schedule table.
 * Sets up the next schedule to be followed by the l_sch_tick function for a certain interface iii.
 * The new schedule will be activated as soon as the current schedule reaches its next schedule entry point.
 * A predefined schedule table, L_NULL_SCHEDULE, shall exist and may be used to stop all transfers on the LIN cluster.
 * @param[in] schedule   schedule table to be set
 * @param[in] entry      defines the starting entry point in the new schedule table.
 *                       The value should be in the range 0 to N if the schedule table has N entries,
 *                       and if entry is 0 or 1 the new schedule table will be started from the beginning.
 * @return  void
 */
void l_sch_set_ma1 (l_schedule_handle schedule, l_u8 entry)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* Fill new schedule table with the values */
    l_m_ScheduleInfo.RequestedSchedule.Schedule = schedule;
    if (entry != 0u) {
        entry--;
    }
    l_m_ScheduleInfo.RequestedSchedule.CurrentEntryPoint = entry;

    /* Set the flag to switch to the new schedule table at start of the next frame slot */
    l_m_ScheduleInfo.isReqScheduleTable = true;

    /* Don't return to interrupted schedule table (application may switch to another
     * schedule table before the end of collision resolving table) */
    l_m_ScheduleInfo.isEventTrigCollision = false;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


#if MA_vLIN_2_1_plus || MA_vISO17987_2016
/** Initialise the LIN Master interface.
 * Initialises the controller. Sets up internal functions such as the baud rate.
 * The default schedule set by the l_ifc_init call will be the L_NULL_SCHEDULE where no frames
 * will be sent and received.
 * This is the first call a user must perform, before using any other interface related LIN
 * API functions.
 * @note The function call shall not interrupt the LIN Master ISR.
 * @return  success status
 * - zero - if the initialisation was successful;
 * - non-zero - if failed.
 */
l_bool l_ifc_init_ma1 (void)
{
    /* (Re)initialise internal variables */
    l_m_IfcInitHelper();

    /* LIN Master driver initialisation  */
    lin_master_Init ((lm_BaudRate_t)MA_LIN_BAUDRATE_MODE);

    /* Enable LIN Master driver IRQ */
    ENABLE_LIN_MASTER_IRQ();

    return false;
}


#else /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */
void l_ifc_init_ma1 (void)
{
    /* (Re)initialise internal variables */
    l_m_IfcInitHelper();
}


/** Connect the interface to the LIN cluster.
 * The call to the will connect the interface to the LIN cluster and enable
 * the transmission of headers and data to the bus.
 * @note The function call shall not interrupt the LIN Master ISR.
 * @return  success status
 * - zero - successful;
 * - non-zero - failed.
 */
l_bool l_ifc_connect_ma1 (void)
{
    /* LIN Master driver initialisation  */
    lin_master_Init ((lm_BaudRate_t)MA_LIN_BAUDRATE_MODE);

    /* Enable LIN Master driver IRQ */
    ENABLE_LIN_MASTER_IRQ();

    return false;
}


/** Disconnect the interface from the LIN cluster.
 * The call to the will disconnect the interface from the LIN cluster and
 * thus disable the interaction with the other nodes in the cluster.
 * @return  success status
 * - zero - successful;
 * - non-zero - failed.
 */
l_bool l_ifc_disconnect_ma1 (void)
{
    /* Stop LIN Master driver immediately */
    lin_master_Stop();

    return false;
}
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */


#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
/** Send GoTo sleep message.
 * This call requests slave nodes on the cluster connected to the interface
 * to enter bus sleep mode by issuing one go to sleep command.
 * The go to sleep command will be scheduled latest when the next schedule entry is due.
 * The l_ifc_goto_sleep will not affect the power mode. It is up to the application to do this.
 * If the go to sleep command was successfully transmitted on the cluster the go to sleep bit
 * will be set in the status register.
 * @return  void
 */
void l_ifc_goto_sleep_ma1 (void)
{
    l_m_IfcStatusHelper.isGotoSleep = true;
}


/** Send Wake Up signal on the LIN bus.
 * The function will transmit one wake up signal. The wake up signal will be transmitted
 * directly when this function is called. It is the responsibility of the application to retransmit
 * the wake up signal according to the wake up sequence.
 * @note The function call shall not interrupt the LIN Master ISR.
 * @return  void
 */
void l_ifc_wake_up_ma1 (void)
{
    /* Process the Wake Up signal */
    if (lin_master_SendWakeUp() == 0u) {
        l_m_IfcStatusHelper.isFrameError = true; /* Set error flag for ifc status */
    }
}


/** Read the LIN Master API status.
 * This function will return the status of the previous communication. The call returns
 * the status word (16 bit value).
 * @return  l_m_IfcStatus word
 */
l_u16 l_ifc_read_status_ma1 (void)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    l_m_IfcStatusHelper.FramesCnt = 0;     /* Clear processed frames counter since the last call */
    l_u16 status = l_m_IfcStatus.word;
    l_m_IfcStatus.word = 0x00;     /* Clear status word as it was sent to application */
    return status;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Send Go to Sleep frame
 * @return  void
 */
static void l_m_ProcessGoToSleep (l_m_CmdProcEvent_t event)
{
    switch (event) {
        case cpe_FrameSlotStart:
            l_m_Driver.Frame.ChecksumType = Classic;
            l_m_Driver.Frame.DataLen = 8;
            l_m_Driver.Frame.PID = 0x3C;
            l_m_Driver.Frame.FrameType = M2S;
            l_m_Driver.Frame.Data = ((l_u8*)ML_MASTER_FRAME_DATA_BUFFER); /* Pointer to reserved area */

            /* Fill Go to Sleep frame with data */
            l_m_Driver.Frame.Data[0] = 0x00;
            l_m_Driver.Frame.Data[1] = 0xFF;
            l_m_Driver.Frame.Data[2] = 0xFF;
            l_m_Driver.Frame.Data[3] = 0xFF;
            l_m_Driver.Frame.Data[4] = 0xFF;
            l_m_Driver.Frame.Data[5] = 0xFF;
            l_m_Driver.Frame.Data[6] = 0xFF;
            l_m_Driver.Frame.Data[7] = 0xFF;

            /* Process the Frame */
            if (lin_master_HandleLinFrame(&l_m_Driver.Frame) == 0u) {
                /* Driver is busy; Frame can't be processed now */
                l_m_IfcStatusHelper.isFrameError = true; /* Set error flag for ifc status */

                /* Don't clear l_m_IfcStatusHelper.isGotoSleep flag -> try again on the next Flame Slot */
            }
            break;
        case cpe_FrameTransmitted:
            /* GoToSleep request has been successfully transmitted */
            l_m_IfcStatusHelper.isGotoSleep = false;         /* Clear request flag */
            l_m_IfcStatusHelper.isGotoSleepConfirmed = true; /* Set confirmation flag */
            break;
        /* Set frame error flag */
        case cpe_FrameError:
            l_m_IfcStatusHelper.isFrameError = true;
            break;

        default:
            /* There is no action supported */
            break;
    }
}


/** The function updates Interface Status word
 * @return  void
 */
static void l_m_UpdateIfcStatus (void)
{
    /*
     * Update ifc status
     */

    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* One more frame was processed because l_m_IfcStatusHelper.isFrameError or l_m_IfcStatusHelper.isFrameConfirmed was updated */
    if ( (l_m_IfcStatusHelper.isFrameError == true) || (l_m_IfcStatusHelper.isFrameConfirmed == true) ) {

        /* PID */
        l_m_IfcStatus.mapped.PID = l_m_Driver.Frame.PID;

        /* Overrun */
        if (l_m_IfcStatusHelper.FramesCnt > 1u) {
            l_m_IfcStatusHelper.FramesCnt = 1;
            l_m_IfcStatus.mapped.Overrun = true;
        }

        /* Error in response */
        if (l_m_IfcStatusHelper.isFrameError == true) {
            l_m_IfcStatus.mapped.ErrorInResponse = true;
            l_m_IfcStatusHelper.isFrameError = false;
        }

        /* Successful transfer */
        if (l_m_IfcStatusHelper.isFrameConfirmed == true) {
            l_m_IfcStatus.mapped.SuccessfulTrans = true;
            l_m_IfcStatusHelper.isFrameConfirmed = false;

            /* Go to Sleep */
            if (l_m_IfcStatusHelper.isGotoSleepConfirmed == true) {
                l_m_IfcStatus.mapped.GoToSleep = true;
                l_m_IfcStatusHelper.isGotoSleepConfirmed = false;
            }
        }
    }
#if MA_vLIN_2_1_plus || MA_vISO17987_2016
    /* Event triggered frame collision */
    l_m_IfcStatus.mapped.EventTriggered = l_m_ScheduleInfo.isEventTrigCollision;
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */


/** Helper initialisation function of the LIN Master interface.
 * Initialises the controller. The default schedule set by the l_ifc_init call will be
 * the L_NULL_SCHEDULE where no frames will be sent and received.
 * @return  void
 */
static void l_m_IfcInitHelper (void)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
    /* Set schedule settings */
    l_m_ScheduleInfo.ActiveSchedule.Schedule = (l_Schedule_t*)&L_NULL_SCHEDULE;
    l_m_ScheduleInfo.ActiveScheduleInstance.Delay = 1;

    /* Set control flags */
    l_m_ScheduleInfo.isEventTrigCollision = false;
    l_m_ScheduleInfo.isReqScheduleTable = false;

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
    /* Set interface Status word and flags */
    l_m_IfcStatus.word = 0x0000;
    l_m_IfcStatusHelper.isFrameConfirmed = false;
    l_m_IfcStatusHelper.isFrameError = false;
    l_m_IfcStatusHelper.FramesCnt = 0;

    l_m_IfcStatusHelper.isGotoSleep = false;
    l_m_IfcStatusHelper.isGotoSleepConfirmed = false;

    /* M2S / S2M frames handlers */
    M2S_handler.Source.byte = 0;
    S2M_handler.Target.byte = 0;

    /* Configuration API */
    ld_m_DiagRequest.isRespExpected = false;
    ld_m_DiagResponse.RSID = DEFAULT_RSID_VALUE;
    ld_m_DiagResponse.error = NO_DIAG_RESP_ERROR;

    ld_m_CfgApiHandler.Status = LD_SERVICE_IDLE;

#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
    ld_m_CheckCfgApiResp20 = CR_LD_SUCCESS;
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}


/** Process current schedule command with the following event.
 * @return void
 */
static void l_m_ProcessScheduleCommand (l_m_CmdProcEvent_t event)
{
#if MA_vLIN_2_0 || MA_vSAE_J2602_2012
    if ( (ld_m_CfgApiHandler.Status == LD_REQUEST_FINISHED)
         && (l_m_ScheduleInfo.ActiveScheduleInstance.ScheduleCommand != sc_SlaveResp)
         ) { /* if slave response is expected but there is no sc_SlaveResp command set after sc_MasterReq */
             /* reset the configuration API status */
        ld_m_CfgApiHandler.Status = LD_SERVICE_IDLE;
    }
#endif /* MA_vLIN_2_0 || MA_vSAE_J2602_2012 */

    switch (l_m_ScheduleInfo.ActiveScheduleInstance.ScheduleCommand) {
        case sc_Frame:
            l_m_ProcessFrame(event);
            break;

#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
#if MA_EN_MASTER_REQ_COMMAND
        case sc_MasterReq:
            (void)ld_ProcessMasterRequest(event);
            break;
#endif /* MA_EN_MASTER_REQ_COMMAND */
#if MA_EN_SLAVE_RESP_COMMAND
        case sc_SlaveResp:
            (void)ld_ProcessSlaveResponse(event);
            break;
#endif /* MA_EN_SLAVE_RESP_COMMAND */
#if MA_EN_ASSIGN_NAD_COMMAND
        case sc_AssignNAD:
            if (event == cpe_FrameSlotStart) {
                l_m_AssignNAD_t *assignNAD = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_assign_NAD(dummy,
                              assignNAD->InitialNAD,
                              assignNAD->SupplierId,
                              assignNAD->FunctionId,
                              assignNAD->NewNAD
                              );
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_ASSIGN_NAD_COMMAND */
#if MA_EN_FREE_FORMAT_COMMAND
        case sc_FreeFormat:
            if (event == cpe_FrameSlotStart) {
                l_m_FreeFormat_t *freeForm = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_m_FreeFormat(freeForm->Data);
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_FREE_FORMAT_COMMAND */

#if MA_vLIN_2_1_plus || MA_vISO17987_2016
#if MA_EN_CONDITIONAL_CHANGE_NAD_COMMAND
        case sc_ConditionalChangeNAD:
            if (event == cpe_FrameSlotStart) {
                l_m_CondChangeNAD_t *condChangeNAD = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_conditional_change_NAD(dummy,
                                          condChangeNAD->NAD,
                                          condChangeNAD->Id,
                                          condChangeNAD->Byte,
                                          condChangeNAD->Mask,
                                          condChangeNAD->Inv,
                                          condChangeNAD->NewNAD
                                          );
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_CONDITIONAL_CHANGE_NAD_COMMAND */
#if MA_EN_DATA_DUMP_COMMAND
        case sc_DataDump:
            if (event == cpe_FrameSlotStart) {
                l_m_DataDump_t *dataDump = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_m_DataDump(dataDump->NAD, dataDump->Data);
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_DATA_DUMP_COMMAND */
#if MA_EN_SAVE_CONFIGURATION_COMMAND
        case sc_SaveConfiguration:
            if (event == cpe_FrameSlotStart) {
                l_m_SaveConfig_t *saveConf = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_save_configuration(dummy, saveConf->NAD);
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_SAVE_CONFIGURATION_COMMAND */
#if MA_EN_ASSIGN_FRAME_ID_RANGE_COMMAND
        case sc_AssignFrameIdRange:
            if (event == cpe_FrameSlotStart) {
                l_m_AssignFrameIdRange_t *assignRange = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_assign_frame_id_range(dummy,
                                         assignRange->NAD,
                                         assignRange->Index,
                                         assignRange->PIDs
                                         );
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_ASSIGN_FRAME_ID_RANGE_COMMAND */
#endif /* MA_vLIN_2_1_plus || MA_vISO17987_2016 */

/* The Master node with version >= 2.0 can support Slave nodes v2.0.
 * The functionality options shall not depend from the Master node version */
#if MA_EN_ASSIGN_FRAME_ID_COMMAND
        case sc_AssignFrameId:
            if (event == cpe_FrameSlotStart) {
                l_m_AssignFrameId_t *assignPID = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_assign_frame_id(dummy,
                                   assignPID->NAD,
                                   assignPID->SupplierId,
                                   assignPID->MessageId,
                                   assignPID->PID
                                   );
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_ASSIGN_FRAME_ID_COMMAND */
#if MA_EN_UNASSIGN_FRAME_ID_COMMAND
        case sc_UnassignFrameId:
            if (event == cpe_FrameSlotStart) {
                l_m_AssignFrameId_t *assignPID = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;
                ld_assign_frame_id(dummy,
                                   assignPID->NAD,
                                   assignPID->SupplierId,
                                   assignPID->MessageId,
                                   UNASSIGN_PID_VALUE
                                   );
            }
            ld_m_ProcessCfgM2S(event);
            break;
#endif /* MA_EN_UNASSIGN_FRAME_ID_COMMAND */

        /* Interleaved GoToSleep request */
        case sc_GoToSleep:
            l_m_ProcessGoToSleep(event);
            break;

#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */

        default:
            /* There is no action supported */
            break;
    }
}


/** Process current frame with the following event.
 * @return void
 */
static void l_m_ProcessFrame (l_m_CmdProcEvent_t event)
{
    l_m_Frame_t *currentFrame = l_m_ScheduleInfo.ActiveScheduleInstance.FunctionArgs;

    switch (currentFrame->FrameType) {
        case mft_UnconditionalFrame:
        {
            l_m_UnconditionalFrame_t *uncondFrame = currentFrame->Frame;

            switch (event) {
                case cpe_FrameSlotStart:
                {
                    l_m_ScheduleInfo.ActiveFrameHandler = uncondFrame->FrameHandler;

                    /* Fill the structure for LIN Master driver */
                    l_m_Driver.Frame.PID = uncondFrame->PID;
                    l_m_Driver.Frame.ChecksumType = uncondFrame->ChecksumType;
                    l_m_Driver.Frame.DataLen = uncondFrame->DataSize;
                    l_m_Driver.Frame.Data = ((l_u8*)ML_MASTER_FRAME_DATA_BUFFER); /* Should be a pointer to reserved memory area */
                    l_m_Driver.Frame.FrameType = uncondFrame->FrameDir;

                    if (l_m_Driver.Frame.FrameType == M2S) { /* Fill the data buffer with packed signal values */
                        (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_FillBuffer);
                    }

                    /* Process the Frame */
                    if (lin_master_HandleLinFrame(&l_m_Driver.Frame) == 0u) {
                        /* Driver is busy; Frame can't be processed now */
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                        l_m_IfcStatusHelper.isFrameError = true; /* Set error flag for ifc status */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                    }
                    break;
                }
                case cpe_FrameReceived:
                    if (l_m_Driver.Frame.DataLen == uncondFrame->DataSize) { /* LM_VARIABLE_RX_FRAME_LENGTH_ENABLED? */
                        (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_UpdateSignals);
                        (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_SetFlags);
                    } else {
                        /* Unconditional frame expected to be full */
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                        l_m_IfcStatusHelper.isFrameError = true;
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                    }
                    break;
                case cpe_FrameTransmitted:
                    (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_SetFlags);
                    break;

                /* Set frame error flag */
                case cpe_RxCollision:
                case cpe_FrameError:
                case cpe_NoResponse:
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                    l_m_IfcStatusHelper.isFrameError = true;
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                    break;

                case cpe_FrameSlotEnd:
                default:
                    /* There is no action supported */
                    break;
            }
            break;
        }

#if !MA_vSAE_J2602_2012 && MA_EN_EVENT_TRIGGERED_FRAME
        case mft_EventTriggeredFrame:
        {
            l_m_EventTriggeredFrame_t *eventTrigFrame = currentFrame->Frame;

            /* LIN2.2: 2.3.3.2: All associated Frames should same length and checksum type */
            l_m_pUnconditionalFrame_t *assocFrames = eventTrigFrame->AssociatedFrames;

            switch (event) {
                case cpe_FrameSlotStart:
                {
                    /* Send header only */

                    /* Fill the structure for LIN Master driver */
                    l_m_Driver.Frame.PID = eventTrigFrame->PID;

                    /* Event-triggered frames have the same Checksum type */
                    l_m_Driver.Frame.ChecksumType = assocFrames[0]->ChecksumType;

                    /* Event-triggered frames have the same data size */
                    l_m_Driver.Frame.DataLen = assocFrames[0]->DataSize;
                    l_m_Driver.Frame.Data = ((l_u8*)ML_MASTER_FRAME_DATA_BUFFER);
                    if (eventTrigFrame->FrameCount > 0u) {
                        l_m_Driver.Frame.FrameType = S2M;
                    } else {
                        l_m_Driver.Frame.FrameType = S2S; /* No expected response; there are no associated frames on which Master is subscribed */
                    }

                    /* Process the Frame */
                    if (lin_master_HandleLinFrame(&l_m_Driver.Frame) == 0u) {
                        /* Driver is busy; Frame can't be processed now */
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                        l_m_IfcStatusHelper.isFrameError = true; /* Set error flag for ifc status */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                    }
                    break;
                }
                case cpe_FrameReceived:
                    if (l_m_Driver.Frame.DataLen == assocFrames[0]->DataSize) {

                        /* Find associated unconditional Frame with replied PID (Data[0]) */
                        l_u8 frameIdx = 0;
                        while ( (frameIdx < eventTrigFrame->FrameCount) && /* Check the limits */
                                (assocFrames[frameIdx]->PID != l_m_Driver.Frame.Data[0]) ) { /* Check the PID */
                            frameIdx++;
                        }

                        l_m_ScheduleInfo.ActiveFrameHandler = assocFrames[frameIdx]->FrameHandler;

                        (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_UpdateSignals);
                        (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_SetFlags);
                    } else {
                        /* Unconditional frame expected to be full */
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                        l_m_IfcStatusHelper.isFrameError = true;
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                    }
                    break;
                case cpe_RxCollision:
                    /* Switch to collision resolver schedule table */
                    l_m_ScheduleInfo.InterruptedSchedule = l_m_ScheduleInfo.ActiveSchedule;
                    l_sch_set_ma1(eventTrigFrame->ColResSchdTable, 0);
                    l_m_ScheduleInfo.isEventTrigCollision = true; /* Set after l_sch_set_ma1; note l_sch_set_ma1() resets this flag */
                    break;

                /* Set frame error flag */
                case cpe_FrameError:
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                    l_m_IfcStatusHelper.isFrameError = true;
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                    break;

                case cpe_FrameTransmitted:
                case cpe_FrameSlotEnd:
                case cpe_NoResponse:
                default:
                    /* There is no action supported */
                    break;
            }
            break;
        }
#endif /* !MA_vSAE_J2602_2012 && MA_EN_EVENT_TRIGGERED_FRAME */

#if (MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012) && MA_EN_SPORADIC_FRAME
        case mft_SporadicFrame:
        {
            l_m_SporadicFrame_t *sporadicFrame = currentFrame->Frame;
            l_m_pUnconditionalFrame_t *assocFrames = sporadicFrame->AssociatedFrames;

            switch (event) {
                case cpe_FrameSlotStart:
                {
                    l_u8 frameIdx = 0;
                    l_bool isUpdated = false;
                    while ( (frameIdx < sporadicFrame->FrameCount) && (isUpdated == false) ) {
                        l_m_FrameHandler_t assocFrameHandler = assocFrames[frameIdx]->FrameHandler;
                        if ( mfhs_isUpdated == assocFrameHandler(mfa_CheckFlags) ) {
                            l_m_ScheduleInfo.ActiveFrameHandler = assocFrameHandler;

                            /* Fill the structure for LIN Master driver */
                            l_m_Driver.Frame.PID = assocFrames[frameIdx]->PID;
                            l_m_Driver.Frame.ChecksumType = assocFrames[frameIdx]->ChecksumType;
                            l_m_Driver.Frame.DataLen = assocFrames[frameIdx]->DataSize;
                            l_m_Driver.Frame.Data = ((l_u8*)ML_MASTER_FRAME_DATA_BUFFER); /* Should be a pointer to reserved memory area */
                            l_m_Driver.Frame.FrameType = M2S; /* Only master node is a publisher of the unconditional frames in the sporadic frame */
                            /* Note: S2M frames which are associated with sporadic frames will create a problem */

                            (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_FillBuffer); /* Fill the data buffer */

                            /* Process the Frame */
                            if (lin_master_HandleLinFrame(&l_m_Driver.Frame) == 0u) {
                                /* Driver is busy; Frame can't be processed now */
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                                l_m_IfcStatusHelper.isFrameError = true; /* Set error flag for ifc status */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                            }

                            isUpdated = true; /* Exit the loop */
                        }
                        frameIdx++;
                    }
                    break;
                }
                case cpe_FrameTransmitted:
                    (void)l_m_ScheduleInfo.ActiveFrameHandler(mfa_SetFlags);
                    break;

                /* Set frame error flag */
                case cpe_FrameError:
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
                    l_m_IfcStatusHelper.isFrameError = true;
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
                    break;

                case cpe_FrameReceived:
                case cpe_FrameSlotEnd:
                case cpe_NoResponse:
                default:
                    /* There is no action supported */
                    break;
            }
            break;
        }
#endif /* (MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012) && MA_EN_SPORADIC_FRAME */

        default:
            /* There is no action supported */
            break;
    }
}


/*
 * Frame handler's helper functions
 */

/** Fill the data buffer from source
 * @param[in]   src     pointer to source
 * @param[in]   size    area size
 */
void l_FillBufferMaster(l_u8 *src, l_u8 size)
{
    for (uint8_t cnt = 0; cnt < size; cnt++) {
        ((l_u8*)ML_MASTER_FRAME_DATA_BUFFER)[cnt] = src[cnt];
    }
}


/*
 * LIN Master driver event functions processed by the LIN API.
 * NOTE: The STD LIN API has no requirements for the lin_master_event_WakeUpDetected() event usage: it's processed by the application.
 */

/** LIN M2S Frame has been successfully transmitted.
 * @param[in]   frame   pointer to transmitter frame
 * @return void
 */
void lin_master_event_FrameTransmitted (lm_Frame_t *frame)
{
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
    l_m_IfcStatusHelper.isFrameConfirmed = true; /* Last frame transmit is fine */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
    l_m_ProcessScheduleCommand(cpe_FrameTransmitted);
    /* Empty body */
    (void)frame;
}


/** LIN S2M Frame has been successfully received.
 * @param[in]   frame   pointer to received frame
 * @return void
 */
void lin_master_event_FrameReceived (lm_Frame_t *frame)
{
#if MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012
    l_m_IfcStatusHelper.isFrameConfirmed = true; /* Last frame receiving is fine */
#endif /* MA_vLIN_2_x || MA_vISO17987_2016 || MA_vSAE_J2602_2012 */
    l_m_ProcessScheduleCommand(cpe_FrameReceived);
    /* Empty body */
    (void)frame;
}


/** LIN Master module error is detected.
 * @param[in]   errorCode   error code value
 * @return void
 */
void lin_master_event_ErrorDetected (lm_ErrorStatus_t errorCode)
{
    switch (errorCode) {
        /* Event-triggered frame collision handler */
        case esRxStopBit:
        case esRxChecksum:
            l_m_ProcessScheduleCommand(cpe_RxCollision);
            break;
        /* Incorrect driver behaviour / state */
        case esUartOverflow:
            l_m_ProcessScheduleCommand(cpe_DriverError);
            break;
        /* LIN specific errors */
        case esTxCollision:
        case esTxStartBit:
            l_m_ProcessScheduleCommand(cpe_FrameError);
            break;
        case esFrameSlotTimeOut:
            l_m_ProcessScheduleCommand(cpe_NoResponse);
            break;
        default:
            /* There is no action supported */
            break;
    }
}

#endif /* LIN_MASTER_API == 1 */
/* EOF */
