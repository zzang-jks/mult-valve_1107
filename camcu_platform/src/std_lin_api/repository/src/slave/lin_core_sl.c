/**
 ******************************************************************************
 * @file    lin_core_sl.c
 * @brief   LIN Slave core API
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
 * @ingroup lin_slave_api
 *
 * @defgroup lin_slave_api LIN Slave API
 * @{
 * @brief   LIN Slave API
 * @details The LIN API is a network software layer that hides the details of a LIN network
 *          configuration (e.g. how signals are mapped into certain frames) for a user making an application
 *          program for an arbitrary ECU. Instead the user will be provided an API, which is focused on
 *          the signals transported on the LIN network. See lin_api_sl.h for information about the interface.
 * @}
 *
 */

#include "lin_api.h"

#if LIN_SLAVE_API == 1

#include "lin_core_sl.h"
#include "lin_cfg_sl.h"

#include "lin_core.h"

//#include "build.h"

/*
 * Declarations
 */

#if SL_vSAE_J2602_2012
l_J2602Status_t s_J2602Status;  /** J2602 status */
#endif /* SL_IS_J2602_VERSION */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
l_s_IfcStatus_t s_ifcStatus;    /** Interface status */
static l_u8 s_ifcOverrunFramesCount;    /** Counter variable for processed frames */
static void l_ifcUpdateOverrun (void);
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

__attribute__((weak)) void l_AutoAddressingStep(l_u8 StepNumber);

__attribute__((weak)) void l_ifc_sleep_entered_callout (ml_SleepReason_t Reason);

#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES

/** Non-unconditional frame type
 * The typedef aim is to cover event-triggered specific behaviour */
typedef enum {
    snuft_False = 0,                /**< False */
    snuft_EventTriggeredFrame,      /**< Event-triggered frame */
    snuft_UncondAssociatedFrame     /**< Unconditional frame associated with event-triggered frame */
} l_s_NonUncondFrameType_t;

/*
 * Event-triggered frame (ETF) or unconditional frame associated with event-triggered frame (AUF)
 * ---------------------
 * is used to notify l_DataTransmitted function that it was transmission of Event Triggered Frame or
 * unconditional frame associated with event-triggered frame.
 *
 * isNonUnconditional is set in l_DataRequest (1) only if MessageIndex < SL_NUMBER_OF_DYNAMIC_MESSAGES
 * and, for ETF only - associated signals have been changed since last transmission.
 *
 * isNonUnconditional is reset in:
 *    2a) l_ErrorDetected (error during transmission of ETF or AUF)
 *    2b) l_DataTransmitted (ETF or AUF has been successfully transmitted)
 *
 * +-------+-----------------------------+
 * |ETF_TX |                             |
 * +-------+-----------------------------+
 *         :                             :
 *      (1)*=======* (2a)                :
 *      (1)*=============================* (2b)
 *         :                             :
 *         :                             l_DataTransmitted
 *         l_DataRequest
 *
 */
static l_s_NonUncondFrameType_t isNonUnconditional; /* True when ETF or AUF is processed */
static l_s_FrameHandler_t NonUncondFrameHandler;    /* ETF or AUF handler function */
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */


#if SL_vSAE_J2602_2012
/* ----------------------------------------------------------------------------
 *  Sets new NAD (correctness of NAD is not checked) and
 *  assign range of FrameIDs to application's MessageIDs depending on NAD
 *
 *  Notes:
 *  1. Functions must be executed in disconnected state of the LIN driver.
 *  2. It's not required to unassign previously assigned FrameIDs, since
 *     they will be overwritten by new FrameIDs during FrameID to Message
 *     Index assignment.
 *
 *                                   LIN    LIN    SAE
 *     NAD       Comment             2.0    2.1   J2602
 *   --------------------------------------------------
 *      0        Goto Sleep Command   -      -      -
 *   0x01..0x5F                       ok     ok     -
 *   0x60..0x6F  SAE J2602 NADs       ok     ok     ok
 *   0x70..0x7D                       ok     ok     -
 *   0x7E        functional node      ok      -     -
 *   0x7F        broadcast            -       -     -
 *   0x80..0xFF  Reserved             -       -     -
 */
void ml_SAE_SetConfiguration (l_u8 nad, l_u8 numMsg)
{
    l_u8 frameID;
    l_u8 i;    /* iterator */

    ml_ConfiguredNAD = nad;

    if (nad < 0x6Eu) {
        frameID = (l_u8)((nad & 0x0Fu) << 2); /* base frame ID for selected NAD/DNN */

        for (i = 0; i < numMsg; i++) {
            if (frameID > 0x37u) {
                break;  /* out of range: incorrect combination NAD vs Number of Messages */
            }
            (void)ml_AssignFrameToMessageID(i, frameID++);
        }
    } else {   /* for NAD = 0x6E or 0x6F */
        for (i = 0; i < numMsg; i++) {
            (void)ml_DisableMessage(i);   /* mark protected IDs as invalid */
        }
    }
}
#endif /* SL_vSAE_J2602_2012 */


/** Initialise the LIN Slave interface.
 * Initialises the controller. Sets up internal functions such as the baud rate.
 * This is the first call a user must perform, before using any other interface related LIN
 * API functions.
 * @note The function call shall not interrupt the LIN Slave ISR.
 * @return  Status:
 * - LIN2.0 and J2602: none
 * - LIN2.1, LIN2.2:
 * -# zero - if the initialisation was successful;
 * -# non-zero - if failed.
 */
#if SL_vLIN_2_0 || SL_vSAE_J2602_2012 || SL_vLIN_1_3
void l_ifc_init_sl1 (void)
#elif SL_vLIN_2_1_plus || SL_vISO17987_2016
l_bool l_ifc_init_sl1 (void)
#else
#error "SL_LIN_VERSION not supported"
#endif /* SL_LIN_VERSION */
{
    /* Initialize MLX LIN Slave API */
    l_mls_Init();

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
    s_ifcOverrunFramesCount = 0;
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
    isNonUnconditional = snuft_False;
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
    ld_DiagInit();  /* initialize diagnostics layer */
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

#if SL_vLIN_2_1_plus || SL_vISO17987_2016
    return (l_bool)ml_Connect();
#elif SL_vSAE_J2602_2012
    /* set Reset flag at start-up */
    s_J2602Status.J2602Error.J2602ErrorByte = 0;
    s_J2602Status.J2602Error.mapped.flagReset = 1;
#endif /* SL_vSAE_J2602_2012 */
}


#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
/** Read the LIN Slave API status.
 * This function will return the status of the previous communication. The call returns
 * the status word (16 bit value)
 * @note Bus (in)activity is the only bit set not according to the frame transmitted or received by the node.
 *       It is checked immediately before returning the status word to the application.
 *       The bit is set, if the node has detected activity on the bus.
 * @note The function call shall not interrupt the LIN Slave ISR; application is responsible to make sure
 *       that the l_ifc_read_status_sl1 IRQ shall be lower than LIN ISR.
 * @return s_ifcStatus word
 */
l_u16 l_ifc_read_status_sl1 (void)
{
    ENTER_STD_LIN_API_ATOMIC_SECTION();
#if SL_vLIN_2_1_plus || SL_vISO17987_2016
    (void)ml_GetState(ML_CLR_LIN_BUS_ACTIVITY);
    if ((ML_SLAVE_BUS_ACTIVITY_STATUS & ML_LIN_BUS_ACTIVITY) != 0U) {
        /* Bus activity detected */
        s_ifcStatus.mapped.BusActivity = true;
    }
#endif /* SL_vLIN_2_1_plus || SL_vISO17987_2016 */
    /* The call is a read-reset call;
     * after the call the status word is set to 0 */
    l_u16 status = s_ifcStatus.word;
    s_ifcOverrunFramesCount = 0u;
    s_ifcStatus.word = 0u;
    return status;
    EXIT_STD_LIN_API_ATOMIC_SECTION();
}
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


#if SL_vLIN_2_0 || SL_vSAE_J2602_2012 || SL_vLIN_1_3
/** Connects the interface sl1 to the LIN cluster and enables the transmission of the data.
 * This command causes the LIN Module to switch from the DISCONNECTED state to the ACTIVE state
 * and starts monitoring the LIN bus for activity.
 * @note This command can be used only from the DISCONNECTED state.
 * @returns l_bool:
 * - zero if the "connect operation" was successful;
 * - non-zero if the "connect operation" failed.
 */
l_bool l_ifc_connect_sl1 (void)
{
    return (l_bool)ml_Connect();
}


/** Disconnects the interface sl1 from the LIN cluster and thus disables the
 * interaction with the other nodes in the cluster.
 * @returns l_bool:
 * - zero if the "disconnect operation" was successful;
 * - non-zero if the "disconnect operation" failed.
 */
l_bool l_ifc_disconnect_sl1 (void)
{
    return (l_bool)ml_Disconnect();
}
#endif /* SL_vLIN_2_0 || SL_vSAE_J2602_2012 || SL_vLIN_1_3 */


#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
/** Send Wake Up signal on the LIN bus.
 * This command will start the transmission of wake up sequence.
 * If there is no activity for 1.5 seconds after the third pulse, the LIN Module will enter the SLEEP state.
 * @note The LIN Module should be in SLEEP or DISCONNECTED state.
 * @note The function call shall not interrupt the LIN Slave ISR.
 * @return  void
 */
void l_ifc_wake_up_sl1 (void)
{
    (void)ml_WakeUp();
}
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


/** Call-back function which is called by the LIN driver to report a reason for entering LIN sleep state.
 * If application specific implementation of l_ifc_sleep_entered_callout is not provided then default (empty)
 * implementation from the LIN API is used. In the default implementation all reported reasons are ignored.
 * @param[in]   Reason    Reason of entering sleep mode.
 * @return void
 */
__attribute__((weak)) void l_ifc_sleep_entered_callout (ml_SleepReason_t Reason)
{
    switch (Reason) {
        /*
         * Go-to-Sleep frame has been received from Master
         */
        case ml_reasonMaster:
        case ml_reasonTimeOut:
        case ml_reasonWakeupAbort:
        case ml_reasonCommand:
        case ml_reasonTimeOutDominant:
        default:
            /*
             * Application specific handlers
             *
             * Note:
             *  Application can implement l_ifc_sleep_entered_callout()
             *  function w/o weak attribute to override this default
             *  implementation.
             */
            break;
    }
}


#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
/** Update ifcStatus overrun flag
 * @return void
 */
static void l_ifcUpdateOverrun (void)
{
    /* Notify driver and check the status */
    if (s_ifcStatus.mapped.Overrun != true) {
        s_ifcOverrunFramesCount++;
        if (s_ifcOverrunFramesCount > 1u) {
            s_ifcStatus.mapped.Overrun = true;
        }
    }
}
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


/*-----------------------------------------------------------------------------
 * LIN Slave driver event functions
 */

/** LIN Slave driver entered a sleep mode.
 * @param[in]   Reason    Reason of entering sleep mode.
 * @return void
 */
void l_LinSleepMode(ml_SleepReason_t Reason)
{

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
    if (Reason == ml_reasonMaster) {
        s_ifcStatus.mapped.GoToSleep = true;

        /*
         * MLX4 FW handles Goto Sleep frame (0x3C, 0x00 ...) automatically
         * and does not report it via l_MessageReceived event.
         * Hence write Protected ID 0x3C explicitly.
         */
        s_ifcStatus.mapped.PID = 0x3C;
        s_ifcStatus.mapped.SuccessfulTrans = true;
    }
    /* else: other reasons of sleep don't required special care */
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

    l_ifc_sleep_entered_callout(Reason);    /* notify application on the sleep reason */
}


/** Non-diagnostic message has been successfully received.
 * A relevant master-to-slave non-diagnostic frame has been received and available
 * in the ML_SLAVE_FRAME_DATA_BUFFER.
 *
 * @param[in]    MessageIndex    Message index of the current Frame
 * @return  void
 * @note The Signals write and Flags update are done atomically.
 * Thus the Signals / Flags API functions are allowed to be used in other interrupts
 * with higher priority then LIN ISR.
 */
void l_MessageReceived(ml_MessageID_t MessageIndex)
{

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
    /*
     * Successful transfer is set if a frame has been
     * transmitted/received without an error
     */
    s_ifcStatus.mapped.SuccessfulTrans = true;
    s_ifcStatus.mapped.PID = ml_GetProtectedID();
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


        if (MessageIndex < SL_NUMBER_OF_DYNAMIC_MESSAGES) {
        /* Get the frame by message index */
        const l_s_Frame_t *currentFrame = (const l_s_Frame_t*)&frameList[MessageIndex];


//#ifdef SUPPORT_MULTI_SLAVE
#if 0

        if (ml_ConfiguredNAD == NAD_VALVE0)
        {
        	if(MessageIndex == SUPPORT_VALVE0_INDEX_START)
        	{
              	/* Get Frame Handler */
        		const l_s_UnconditionalFrame_t *uncondFrame = currentFrame->Frame;
        		l_s_FrameHandler_t ProcessFrame = uncondFrame->FrameHandler;

        		/* Actions on the frame */
        		(void)ProcessFrame(sfa_UpdateSignals); /* Use new signals values */
        		(void)ProcessFrame(sfa_SetFlags);   /* Mark corresponding signal's flags as received */
                l_ifcUpdateOverrun();
        	}
        	else
            {
            		(void)ml_DiscardFrame();
            }

        }
        if (ml_ConfiguredNAD == NAD_VALVE1)
        {
        	if(MessageIndex == SUPPORT_VALVE1_INDEX_START)
        	{
              	/* Get Frame Handler */
        		const l_s_UnconditionalFrame_t *uncondFrame = currentFrame->Frame;
        		l_s_FrameHandler_t ProcessFrame = uncondFrame->FrameHandler;

        		/* Actions on the frame */
        		(void)ProcessFrame(sfa_UpdateSignals); /* Use new signals values */
        		(void)ProcessFrame(sfa_SetFlags);   /* Mark corresponding signal's flags as received */
                l_ifcUpdateOverrun();
        	}
        	else
            {
            		(void)ml_DiscardFrame();
            }

        }
        if (ml_ConfiguredNAD == NAD_VALVE2)
        {
        	if(MessageIndex == SUPPORT_VALVE2_INDEX_START)
        	{
              	/* Get Frame Handler */
        		const l_s_UnconditionalFrame_t *uncondFrame = currentFrame->Frame;
        		l_s_FrameHandler_t ProcessFrame = uncondFrame->FrameHandler;

        		/* Actions on the frame */
        		(void)ProcessFrame(sfa_UpdateSignals); /* Use new signals values */
        		(void)ProcessFrame(sfa_SetFlags);   /* Mark corresponding signal's flags as received */
                l_ifcUpdateOverrun();
        	}
        	else
            {
            		(void)ml_DiscardFrame();
            }

        }
        if (ml_ConfiguredNAD == NAD_VALVE3)
        {
        	if(MessageIndex == SUPPORT_VALVE3_INDEX_START)
        	{
              	/* Get Frame Handler */
        		const l_s_UnconditionalFrame_t *uncondFrame = currentFrame->Frame;
        		l_s_FrameHandler_t ProcessFrame = uncondFrame->FrameHandler;

        		/* Actions on the frame */
        		(void)ProcessFrame(sfa_UpdateSignals); /* Use new signals values */
        		(void)ProcessFrame(sfa_SetFlags);   /* Mark corresponding signal's flags as received */
                l_ifcUpdateOverrun();
        	}
        	else
            {
            		(void)ml_DiscardFrame();
            }

        }
#else /*SUPPORT_MULTI_SLAVE*/

		/* Get Frame Handler */
		const l_s_UnconditionalFrame_t *uncondFrame = currentFrame->Frame;
		l_s_FrameHandler_t ProcessFrame = uncondFrame->FrameHandler;

		/* Actions on the frame */
		(void)ProcessFrame(sfa_UpdateSignals); /* Use new signals values */
		(void)ProcessFrame(sfa_SetFlags);   /* Mark corresponding signal's flags as received */


#if SL_vSAE_J2602_2012
        mlu_SAE_MessageReceived (MessageIndex);
#endif /* SL_vSAE_J2602_2012 */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
//        l_ifcUpdateOverrun();
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */
#endif /*SUPPORT_MULTI_SLAVE*/
    } else {
        /* Unexpected frame -> ignore */
    }
}


/** Header of the relevant non-diagnostic slave-to-master frame has been received.
 * @param[in]    MessageIndex    Message index of the current Frame
 * @return  void
 */
void l_DataRequest (ml_MessageID_t MessageIndex)
{
    if (MessageIndex < SL_NUMBER_OF_DYNAMIC_MESSAGES) {
        /* Non-diagnostic message */
        /* Get the frame by message index */
        const l_s_Frame_t *currentFrame = (const l_s_Frame_t*)&frameList[MessageIndex];

        /* Unconditional frame */
        if (currentFrame->FrameType == sft_UnconditionalFrame) {
            /* Get Frame Handler */
            const l_s_UnconditionalFrame_t *uncondFrame = currentFrame->Frame;
            const l_s_FrameHandler_t ProcessFrame = uncondFrame->FrameHandler;

#ifdef SUPPORT_MULTI_SLAVE

            if (ml_ConfiguredNAD == NAD_VALVE0)
            {
//            	if((MessageIndex == SUPPORT_VALVE0_INDEX_START + 1) || (MessageIndex == SUPPORT_VALVE0_INDEX_START + 2))
              	if(MessageIndex == SUPPORT_VALVE0_INDEX_START + 1)
            	{
                    /* Actions on the frame */
                    (void)ProcessFrame(sfa_FillBuffer);          /* Fill the data buffer */
                    (void)ProcessFrame(sfa_SetFlags);            /* Consider Signals as sent */
                    (void)ml_DataReady(ML_END_OF_TX_ENABLED);    /* Notify driver */
            	}
            	else
                {
                		(void)ml_DiscardFrame();
                }

            }
            if (ml_ConfiguredNAD == NAD_VALVE1)
            {
//            	if((MessageIndex == SUPPORT_VALVE1_INDEX_START + 1) || (MessageIndex == SUPPORT_VALVE1_INDEX_START + 2))
               	if(MessageIndex == SUPPORT_VALVE1_INDEX_START + 1)
            	{
                    /* Actions on the frame */
                    (void)ProcessFrame(sfa_FillBuffer);          /* Fill the data buffer */
                    (void)ProcessFrame(sfa_SetFlags);            /* Consider Signals as sent */
                    (void)ml_DataReady(ML_END_OF_TX_ENABLED);    /* Notify driver */
            	}
            	else
                {
                		(void)ml_DiscardFrame();
                }

            }
            if (ml_ConfiguredNAD == NAD_VALVE2)
            {
//            	if((MessageIndex == SUPPORT_VALVE2_INDEX_START + 1) || (MessageIndex == SUPPORT_VALVE2_INDEX_START + 2))
               	if(MessageIndex == SUPPORT_VALVE2_INDEX_START + 1)
            	{
                    /* Actions on the frame */
                    (void)ProcessFrame(sfa_FillBuffer);          /* Fill the data buffer */
                    (void)ProcessFrame(sfa_SetFlags);            /* Consider Signals as sent */
                    (void)ml_DataReady(ML_END_OF_TX_ENABLED);    /* Notify driver */
            	}
            	else
                {
                		(void)ml_DiscardFrame();
                }

            }
			if (ml_ConfiguredNAD == NAD_VALVE3)
            {
//            	if((MessageIndex == SUPPORT_VALVE3_INDEX_START + 1) || (MessageIndex == SUPPORT_VALVE3_INDEX_START + 2))
               	if(MessageIndex == SUPPORT_VALVE3_INDEX_START + 1)
            	{
                    /* Actions on the frame */
                    (void)ProcessFrame(sfa_FillBuffer);          /* Fill the data buffer */
                    (void)ProcessFrame(sfa_SetFlags);            /* Consider Signals as sent */
                    (void)ml_DataReady(ML_END_OF_TX_ENABLED);    /* Notify driver */
            	}
            	else
                {
                		(void)ml_DiscardFrame();
                }

            }
			if (ml_ConfiguredNAD == NAD_VALVE4)
            {
//            	if((MessageIndex == SUPPORT_VALVE3_INDEX_START + 1) || (MessageIndex == SUPPORT_VALVE3_INDEX_START + 2))
               	if(MessageIndex == SUPPORT_VALVE4_INDEX_START + 1)
            	{
                    /* Actions on the frame */
                    (void)ProcessFrame(sfa_FillBuffer);          /* Fill the data buffer */
                    (void)ProcessFrame(sfa_SetFlags);            /* Consider Signals as sent */
                    (void)ml_DataReady(ML_END_OF_TX_ENABLED);    /* Notify driver */
            	}
            	else
                {
                		(void)ml_DiscardFrame();
                }

            }


#else	/*SUPPORT_MULTI_SLAVE*/

        /* Actions on the frame */
        (void)ProcessFrame(sfa_FillBuffer);          /* Fill the data buffer */
        (void)ProcessFrame(sfa_SetFlags);            /* Consider Signals as sent */

#if SL_vSAE_J2602_2012
        l_u8 api_info = mlu_SAE_SetStatusApiInfo(MessageIndex); /* call User Application */
        (void)ml_SAE_DataReady(api_info);            /* Notify driver */
#else
        (void)ml_DataReady(ML_END_OF_TX_ENABLED);    /* Notify driver */

#endif /* SL_vSAE_J2602_2012 */
#endif	/*SUPPORT_MULTI_SLAVE*/
        }

#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
        /* Associated unconditional frame */
        else if (currentFrame->FrameType == sft_UncondAssociatedFrame) {
            /* Get Frame Handler */
            const l_s_AssociatedUncondFrame_t *aUncondFrame = currentFrame->Frame;
            const l_s_FrameHandler_t ProcessFrame = aUncondFrame->FrameHandler;

            isNonUnconditional = snuft_UncondAssociatedFrame;
            NonUncondFrameHandler = ProcessFrame;       /* Flags will be updated after successful transmit */

            /* Actions on the frame */
            (void)ProcessFrame(sfa_FillBuffer);         /* Fill the data buffer */
            (void)ml_DataReady(ML_END_OF_TX_ENABLED);   /* Notify driver */
        }
        /* Event triggered frame */
        else if (currentFrame->FrameType == sft_EventTriggeredFrame) {
            /* Get Frame Handler */
            const l_s_EventTriggeredFrame_t *eventFrame = currentFrame->Frame;
            const l_s_AssociatedUncondFrame_t *aUncondFrame = eventFrame->AssociatedFrame;
            const l_s_FrameHandler_t ProcessFrame = aUncondFrame->FrameHandler;

            /* If some signals are updated -> send associated unconditional frame */
            if (ProcessFrame(sfa_CheckFlags) == sfhs_isUpdated) {
                isNonUnconditional = snuft_EventTriggeredFrame;
                NonUncondFrameHandler = ProcessFrame;       /* Flags will be updated after successful transmit */
                (void)ProcessFrame(sfa_FillAsBuffer);       /* Fill the data buffer */
                (void)ml_DataReady(ML_END_OF_TX_ENABLED);   /* Notify driver */
            }
            /* If no updated signals -> discard frame */
            else {
                (void)ml_DiscardFrame();
            }
        }
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */
        /* Unexpected frame type -> ignore */
        else {
            (void)ml_DiscardFrame();
        }
    }
    /* Unexpected message index -> ignore */
    else {
        (void)ml_DiscardFrame();
    }
}


/** An error was detected on the LIN bus.
 * @param[in]   Error   error code
 * @return  void
 */
void l_ErrorDetected(ml_LinError_t Error)
{

#if SL_vSAE_J2602_2012
    /* The J2602 only specific action to the errors */
    switch (Error) {
        case ml_erIdParity:
            s_J2602Status.J2602Error.mapped.flagIDParityError = 1;
            break;

        case ml_erCheckSum:
            s_J2602Status.J2602Error.mapped.flagChecksumError = 1;
            break;

        case ml_erStopBitTX:
        case ml_erDataFraming:
            s_J2602Status.J2602Error.mapped.flagByteFramingError = 1;
            break;

        case ml_erBit:
        case ml_erSynchField:
            s_J2602Status.J2602Error.mapped.flagDataError = 1;
            break;

        default:
            /* There is no J2602 only specific action for other errors */
            break;
    }
#endif /* SL_vSAE_J2602_2012 */

    switch (Error) {
        case ml_erDataFraming:
        case ml_erBreakDetected:
        case ml_erCheckSum:
#if SL_vLIN_1_3
            /* empty handler for LIN1.3 */
#elif !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
            if (isNonUnconditional != snuft_EventTriggeredFrame) {
                l_bool_wr_response_error(true);
                s_ifcStatus.mapped.ErrorInResponse = true;
                s_ifcStatus.mapped.PID = ml_GetProtectedID();
#if SL_vLIN_2_x || SL_vISO17987_2016
                l_ifcUpdateOverrun();
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 */
            }
#else /* SL_vSAE_J2602_2012 || (SL_EN_EVENT_TRIGGERED_FRAMES == 0) */
#if !SL_vSAE_J2602_2012
            l_bool_wr_response_error(true);
#endif /* !SL_vSAE_J2602_2012 */
            s_ifcStatus.mapped.ErrorInResponse = true;
            s_ifcStatus.mapped.PID = ml_GetProtectedID();
#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
            l_ifcUpdateOverrun();
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */
#endif /* SL_vSAE_J2602_2012 || (SL_EN_EVENT_TRIGGERED_FRAMES == 0) */
            break;

        case ml_erBit:
        case ml_erStopBitTX:
#if SL_vLIN_1_3
            /* empty handler for LIN1.3 */
#elif !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
            if (isNonUnconditional != snuft_EventTriggeredFrame) {
                /*
                 * Clear flag which indicates that response_error signal
                 * is being transmitted
                 */
                l_flg_clr_response_error();

                l_bool_wr_response_error(true);
                s_ifcStatus.mapped.ErrorInResponse = true;
                s_ifcStatus.mapped.PID = ml_GetProtectedID();
#if SL_vLIN_2_x || SL_vISO17987_2016
                l_ifcUpdateOverrun();
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 */
            }
#else /* SL_vSAE_J2602_2012 || (SL_EN_EVENT_TRIGGERED_FRAMES == 0) */
#if !SL_vSAE_J2602_2012
            /*
             * Clear flag which indicates that response_error signal
             * is being transmitted
             */
            l_flg_clr_response_error();

            l_bool_wr_response_error(true);
#endif /* !SL_vSAE_J2602_2012 */
            s_ifcStatus.mapped.ErrorInResponse = true;
            s_ifcStatus.mapped.PID = ml_GetProtectedID();
#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
            l_ifcUpdateOverrun();
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */
#endif /* SL_vSAE_J2602_2012 || (SL_EN_EVENT_TRIGGERED_FRAMES == 0) */
            break;

        default:
            /* Ignore other errors */
            break;
    }

#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
    isNonUnconditional = snuft_False;
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */
}


/** Frame has been successfully transmitted.
 * @return  void
 */
void l_DataTransmitted(ml_MessageID_t MessageIndex)
{
    (void)MessageIndex;
#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
    /*
     * Successful transfer is set if a frame has been
     * transmitted/received without an error
     */
    s_ifcStatus.mapped.SuccessfulTrans = true;
    s_ifcStatus.mapped.PID = ml_GetProtectedID();
    l_ifcUpdateOverrun();
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

#if SL_vLIN_2_x || SL_vISO17987_2016
    /*
     * The response_error flag is set when Response Error signal is copied to
     * frame buffer for transmission in the current frame. If an error occurred
     * during transmission, the flag is cleared in l_ErrorDetected handler.
     */
    if (l_flg_tst_response_error()) {
        l_flg_clr_response_error();

        /* response error has been successfully transmitted */
        l_bool_wr_response_error(false);
    }
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 */

#if !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES
    if (isNonUnconditional != snuft_False) { /* For both ETF and AUF */
        (void)NonUncondFrameHandler(sfa_SetFlags);
        isNonUnconditional = snuft_False;
    }
#endif /* !SL_vSAE_J2602_2012 && SL_EN_EVENT_TRIGGERED_FRAMES */

#if SL_vSAE_J2602_2012
    /*
     * Frame successfully transmitted. If it was a frame with SEA J2602
     * Status Byte, then clear current Error state, which was successfully
     * reported in this frame
     */
    if (s_J2602Status.J2602HasStatusFlag == 1u) {
        s_J2602Status.J2602HasStatusFlag = 0;
        s_J2602Status.J2602Error.J2602ErrorByte &= ~(1u << s_J2602Status.J2602CurrentError);
    }

    /* Notify user application about end of transmission */
    mlu_SAE_DataTransmitted();
#endif /* SL_vSAE_J2602_2012 */
}


/** A step is sent by the LIN Module during the auto addressing
 * @param[in]   StepNumber  current auto-addressing step number
 * @return  void
 */
__attribute__((weak)) void l_AutoAddressingStep(l_u8 StepNumber)
{
    (void)StepNumber;   /* unused parameter */
}


/*
 * Frame handler's helper functions
 */

/** Fill the data buffer from source
 * @param[in]   src     pointer to source
 * @param[in]   size    area size
 */
void l_FillBufferSlave(l_u8 *src, l_u8 size)
{
    for (l_u8 cnt = 0; cnt < size; cnt++) {
        ((l_u8*)ML_SLAVE_FRAME_DATA_BUFFER)[cnt] = src[cnt];
    }
}

#endif /* LIN_SLAVE_API == 1 */
/* EOF */
