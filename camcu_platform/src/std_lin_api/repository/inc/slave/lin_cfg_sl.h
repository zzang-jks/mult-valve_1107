/**
 ******************************************************************************
 * @file    lin_cfg_sl.h
 * @brief   LIN Slave node configuration and identification API
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
 * @details The LIN node configuration and identification API.
 *
 * @ingroup lin_slave_api
 *
 */

#ifndef LIN_CFG_SL_H
#define LIN_CFG_SL_H

#ifndef LIN_API_H
/* For Doxygen only */
#include <lin_api.h>
#endif

/*-----------------------------------------------------------------------------
 * Macro
 */
#define RSID(sid) ((l_u8)(sid)+0x40u)


/*-----------------------------------------------------------------------------
 * Type Definition
 */

#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
    SL_EN_SAVE_CONFIG_FUNC)
/* return values for ld_read_configuration */
enum {
    LD_READ_OK,
    LD_LENGTH_TOO_SHORT
};

/* return values for ld_set_configuration */
enum {
    LD_SET_OK,
    LD_LENGTH_NOT_CORRECT,
    LD_DATA_ERROR
};
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
/* The SID values used for configuration and identification API by different LIN versions */
enum {
    SID_AssignNad = 0xB0u,
    SID_AssignFrameId = 0xB1u,
    SID_ReadById = 0xB2u,
    SID_ConditionaChangeNad = 0xB3u,
    SID_DataDump = 0xB4u,
    SID_J2602_TargetedReset = 0xB5u,
    SID_SaveConfiguration = 0xB6u,
    SID_AssignFrameIdRange = 0xB7u
};

/* Special RSID values */
enum {
    RSID_NegativeResponse = 0x7Fu
};
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */


/*-----------------------------------------------------------------------------
 * Function prototypes
 */

#if SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012
extern void ld_DiagInit (void);
#endif /* SL_vLIN_2_x || SL_vISO17987_2016 || SL_vSAE_J2602_2012 */

#if SL_vSAE_J2602_2012 && SL_EN_ASSIGN_NAD_SERVICE
extern void ml_SAE_SetConfiguration (l_u8 nad, l_u8 numMsg);
#endif /* SL_vSAE_J2602_2012 && SL_EN_ASSIGN_NAD_SERVICE */


/*-----------------------------------------------------------------------------
 *  Externs
 */

#if ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && \
    SL_EN_SAVE_CONFIG_FUNC)
extern l_u8 node_configuration[SL_NUMBER_OF_DYNAMIC_MESSAGES];
#endif /* ((SL_vLIN_2_1_plus || SL_vISO17987_2016) && SL_EN_SAVE_CONFIG_SERVICE) || ((SL_vLIN_2_0 || SL_vSAE_J2602_2012) && SL_EN_SAVE_CONFIG_FUNC) */

#if SL_vLIN_2_0 || SL_vSAE_J2602_2012
extern const l_u16 MID_list[SL_NUMBER_OF_DYNAMIC_MESSAGES];
#endif /* SL_vLIN_2_0 || SL_vSAE_J2602_2012 */


#endif /* LIN_CFG_SL_H */

/* EOF */
