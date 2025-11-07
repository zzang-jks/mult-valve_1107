/**
 * @file
 * @brief Diagnostic (HW protections) handling.
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

#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

/* ---------------------------
 * Public Type Definitions
 * --------------------------- */

/** Main temperature sensor selection */
typedef enum tsensor
{
    tsensorVdda = 2u, /**< Temperature Sensor is VDDA */
} tsensor_t;

/** VDDA Detection level selection */
typedef enum vddauv
{
    vddauvLow = 0u,  /**< VDDA UV detection level = 2.85V */
    vddauvHigh = 1u, /**< VDDA UV detection level = 3.00V */
} vddauv_t;

/** VS Under Voltage detection level selection */
typedef enum vsuv
{
    vsuv4V = 0u, /**< VS UV detection level = 4V */
    vsuv5V = 1u, /**< VS UV detection level = 5V */
    vsuv6V = 2u, /**< VS UV detection level = 6V */
    vsuv7V = 3u, /**< VS UV detection level = 7V */
    vsuv8V = 4u, /**< VS UV detection level = 8V */
    vsuv9V = 5u, /**< VS UV detection level = 9V */
} vsuv_t;

/** VS Over Voltage detection level selection */
typedef enum vsov
{
    vsov22V = 0u, /**< VS OV detection level = 22V */
    vsov24V = 1u, /**< VS OV detection level = 24V */
    vsov40V = 2u, /**< VS OV detection level = 40V */
    vsovOff = 3u, /**< VS OV detection level Off */
} vsov_t;

/* ---------------------------
 * Public Function Definitions
 * --------------------------- */

void diagnostic_Init(void);
void diagnostic_Close(void);
void diagnostic_Start(void);
bool diagnostic_IsUVPending(void);
bool diagnostic_IsOVPending(void);
bool diagnostic_IsOVTPending(void);
bool diagnostic_IsOVCPending(void);
bool diagnostic_IsVDSPending(void);
void DIAGNOSTIC_Reset(void);

/* ---------------------------
 * Public Variables
 * --------------------------- */

#endif /* DIAGNOSTIC_H_ */

/* EOF */
