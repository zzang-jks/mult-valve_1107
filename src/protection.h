/**
 * @file
 * @brief Protections detection handling
 *
 * @internal
 *
 * @copyright (C) 2020 Melexis N.V.
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
 */

#ifndef PROTECTION_H_
#define PROTECTION_H_

/* ---------------------------
 * Public Defines
 * --------------------------- */

/** [10mV] */
#define C_APPL_UVOLT 700
/** [10mV] */
#define C_APPL_OVOLT 1800
/** [C] */
#define C_APPL_OTEMP 120

/** [C] Chip Over-temperature level */
#define C_CHIP_OVERTEMP_LEVEL 150
/** [C] Temperature hysteresis */
#define C_TEMPERATURE_HYS 3
/** [10mV] Voltage hysteric */
#define C_VOLTAGE_HYS 50U

/* ---------------------------
 * Public Type Definitions
 * --------------------------- */

/** Short-circuit error */
typedef enum ErrShort_e
{
    C_ERR_SHORT_NO = 0U, /**< No short error detected */
    C_ERR_SHORT_OC,      /**< Short to VBAT */
    C_ERR_SHORT_VDS,     /**< Short to GND */
    C_ERR_SHORT_NR_OF    /**< Size of enum */
} ErrShort_t;

/** Voltage error*/
typedef enum ErrVoltage_e
{
    C_ERR_VOLTAGE_IN_RANGE = 0U, /**< Module within operational voltage range */
    C_ERR_VOLTAGE_UNDER,         /**< Under software voltage threshold */
    C_ERR_VOLTAGE_UNDER_HW,      /**< Under hardware voltage threshold */
    C_ERR_VOLTAGE_OVER,          /**< Over software voltage threshold */
    C_ERR_VOLTAGE_OVER_HW,       /**< Over hardware voltage threshold */
    C_ERR_VOLTAGE_NR_OF          /**< Size of enum */
} ErrVoltage_t;

/** Temperature error */
typedef enum ErrTemp_e
{
    C_ERR_TEMP_NO = 0U, /**< Module within operational temperature range */
    C_ERR_TEMP_WARNING, /**< Warning level */
    C_ERR_TEMP_ALARM,   /**< Alarm level */
    C_ERR_TEMP_NR_OF    /**< Size of enum */
} ErrTemp_t;

/* ---------------------------
 * Public Function Definitions
 * --------------------------- */

void protection_Init(void);
void protection_Close(void);
void protection_Start(void);
void protection_Stop(void);
void protection_CheckShort(void);
void protection_CheckSupplyMotorVoltage(void);
void protection_CheckChipTemperature(void);
void protection_Task(void);

/* ---------------------------
 * Public Variables
 * --------------------------- */
extern ErrVoltage_t g_e8ErrorVoltage;
extern ErrTemp_t g_e8ErrorOverTemperature;
extern ErrShort_t g_e8ShortOcc;
extern uint8_t g_e8OverCurrent;
static INLINE ErrVoltage_t protection_GetErrorVoltage(void)
{

    return g_e8ErrorVoltage;
}

static INLINE ErrTemp_t protection_GetErrorOverTemperature(void)
{

    return g_e8ErrorOverTemperature;
}

static INLINE ErrShort_t protection_GetShortOcc(void)
{
    extern ErrShort_t g_e8ShortOcc;
    return g_e8ShortOcc;
}

#endif /* PROTECTION_H_ */

/* EOF */
