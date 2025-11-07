/**
 * @file
 * @brief The filter_avg (moving-average filter) library definitions.
 * @internal
 *
 * @copyright (C) 2019 Melexis N.V.
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
 * This file contains the definitions of the filter_avg (moving-average filter) library.
 */

#ifndef FILTER_AVG_H
#define FILTER_AVG_H

#include <stdbool.h>

/**
 * The structure of the FILTER_AVG object
 */
typedef struct FILTER_AVG_Object_s
{
    uint16_t u16MovAvg;    /**< Moving average */
    uint32_t u32MovAvgxN;  /**< Moving average filter */
    uint16_t* pu16Raw;     /**< Buffer address */
    uint16_t u16Size;      /**< Number of elements @warning should be a power of 2 */
    uint16_t u16RawIdx;    /**< Index */
} FILTER_AVG_Object_t;

/* ---------------------------
 * Public Function Definitions
 * --------------------------- */

bool FILTER_AVG_Init(FILTER_AVG_Object_t* const handler);
void FILTER_AVG_CalcMovAvg(FILTER_AVG_Object_t* const handler, uint16_t a_u16NewValue);

static __inline__ __attribute__ ((__always_inline__))
uint16_t get_u16MovAvg(const FILTER_AVG_Object_t* const handler)
{
    return handler->u16MovAvg;
}
#endif /* FILTER_AVG_H */

/* EOF */
