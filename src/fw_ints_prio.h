/**
 * @file
 * @brief Interrupt priority settings
 * @internal
 *
 * @copyright (C) 2021-2022 Melexis N.V.
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
 */

#ifndef FW_INTS_PRIO_H_
#define FW_INTS_PRIO_H_

/** Enable CPU system mode */
#define CPU_SYSTEM_MODE (0 << 3)

/** Enable CPU user mode */
#define CPU_USER_MODE (1 << 3)

/** Interrupt priority level for STIMER: Simple Timer interrupt [3, 6] */
#define PRIO_STIMER_INT 6
/** Interrupt CPU mode for STIMER: Simple Timer interrupt [CPU_SYSTEM_MODE/CPU_USER_MODE] */
#define MODE_STIMER_INT CPU_SYSTEM_MODE

/** Interrupt priority level for COLIN_LIN: LIN interrupt [3, 6] */
#define PRIO_COLIN_LIN_INT MLS_IRQ_PRIO
/** Interrupt CPU mode for COLIN_LIN: LIN interrupt [CPU_SYSTEM_MODE/CPU_USER_MODE] */
#define MODE_COLIN_LIN_INT CPU_SYSTEM_MODE

/** Interrupt priority level for UDFR:  [3, 6] */
#define PRIO_UDFR_INT 5
/** Interrupt CPU mode for UDFR:  [CPU_SYSTEM_MODE/CPU_USER_MODE] */
#define MODE_UDFR_INT CPU_SYSTEM_MODE

/** Interrupt priority level for UDTF:  [3, 6] */
#define PRIO_UDTF_INT 5
/** Interrupt CPU mode for UDTF:  [CPU_SYSTEM_MODE/CPU_USER_MODE] */
#define MODE_UDTF_INT CPU_SYSTEM_MODE

/** Interrupt priority level for ADC_SAR: ADC interrupt [3, 6] */
#define PRIO_ADC_SAR_INT 4
/** Interrupt CPU mode for ADC_SAR: ADC interrupt [CPU_SYSTEM_MODE/CPU_USER_MODE] */
#define MODE_ADC_SAR_INT CPU_SYSTEM_MODE

/** Interrupt priority level for OV_VS:  [3, 6] */
#define PRIO_OV_VS_INT 3
/** Interrupt CPU mode for OV_VS:  [CPU_SYSTEM_MODE/CPU_USER_MODE] */
#define MODE_OV_VS_INT CPU_SYSTEM_MODE

/** Interrupt priority level for UV_VS:  [3, 6] */
#define PRIO_UV_VS_INT 3
/** Interrupt CPU mode for OV_VS:  [CPU_SYSTEM_MODE/CPU_USER_MODE] */
#define MODE_UV_VS_INT CPU_SYSTEM_MODE

#define PRIO_PPM_ERR_INT 3

#endif /* FW_INTS_PRIO_H_ */

/* EOF */
