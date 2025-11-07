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

#include <string.h>
#include <syslib.h>
#include <mathlib.h>
#include "filter_avg.h"

/* ---------------------------------------------
 * Local Defines
 * --------------------------------------------- */

/* ---------------------------------------------
 * Local Variables
 * --------------------------------------------- */

/* ---------------------------------------------
 * Global Functions
 * --------------------------------------------- */

/**
 * @brief Initialize, clear all variables
 * @param a_pHandler The handle (pointer to FILTER_AVG_Object_t structure) to the AVG object
 * @returns  true  in case of success
 */
bool FILTER_AVG_Init(FILTER_AVG_Object_t* const a_pHandler)
{
    a_pHandler->u16MovAvg = 0u;
    a_pHandler->u16RawIdx = 0u;
    a_pHandler->u32MovAvgxN = 0u;

    if ((a_pHandler->u16Size < 2) || (a_pHandler->u16Size & (a_pHandler->u16Size - 1)))
    {
        return false;  /* size shall be in a powers of 2, i.e. 8, 16, 32 etc */
    }

    if (&a_pHandler->pu16Raw[0] == NULL)
    {
        return false;
    }

    memset(&a_pHandler->pu16Raw[0], 0, a_pHandler->u16Size * 2);

    return true;
}

/**
 * @brief add a value in the AVG buffer and calculates the moving average
 * @param a_pHandler The handle (pointer to FILTER_AVG_Object_t structure) to the FILTER_AVG object
 * @param a_u16NewValue A new value to add to the moving-average buffer
 */
void FILTER_AVG_CalcMovAvg(FILTER_AVG_Object_t* const a_pHandler, uint16_t a_u16NewValue)
{
    uint16_t *pu16Element = &a_pHandler->pu16Raw[a_pHandler->u16RawIdx];
    a_pHandler->u16RawIdx = (a_pHandler->u16RawIdx + 1u) & (a_pHandler->u16Size - 1u);  /* Increment index */
    uint16_t u16Oldest = *pu16Element;
    a_pHandler->u32MovAvgxN -= u16Oldest;  /* Subtract oldest raw element */
    a_pHandler->u32MovAvgxN += (uint32_t)a_u16NewValue;  /* Add newest raw element */
    *pu16Element = (uint16_t)a_u16NewValue;  /* Overwrite oldest with newest element */
    a_pHandler->u16MovAvg = divU16_U32byU16(a_pHandler->u32MovAvgxN, a_pHandler->u16Size);
}

/* EOF */
