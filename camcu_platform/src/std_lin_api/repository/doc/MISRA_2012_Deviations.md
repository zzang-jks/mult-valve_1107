## The results related to the **LDF-dependent LIN API** check with Coverity for MISRA 2012

### Intentional MISRA violations reasoning

#### MISRA-rigor: required

- *Rule 11.3:*
  - *Violation:* A cast is performed between a pointer to object type and a pointer to a different object type.
  - *Reasoning:* The target pointer data type can represent the data from input buffer LinFrameDataBuffer correctly.
- *Rule 11.8:*
  - *Violation:* A cast removes volatile qualification from the type pointed to by a pointer.
  - *Reasoning:* The access to the buffer through the pointer is allowed in optimized way as there is no task
  which can update the content of the buffer at the same time.

Example (common for Rule 11.3 and Rule 11.8):
```
extern volatile uint8_t LinFrameDataBuffer[8] __attribute__((aligned(2)));

typedef struct ATTR_PACKED {
    l_u16 i11  : 11;
    l_u16 i12  : 12;
    l_u16 i13  : 13;
    l_u16 i14  : 14;
    l_u8 unused50_0  : 6;
    l_u8 unused56_0  : 8;
} l_sl1_M2S_2_data_t;

l_sl1_M2S_2_data_t *M2S_2_data = (l_sl1_M2S_2_data_t*)LinFrameDataBuffer;
```

- *Rule 20.7:*
  - *Violation:* Macro parameter expands into an expression without being wrapped by parentheses.
  - *Reasoning:* The `'.'` operator doesn't allow the parentheses used around the operands (`bufName` & `flagName`).

Example:
```
#define L_FLAGS(bufName, flagName, flagType, baseName)                          \
/** @name The flagType "baseName" flag interaction */                           \
/** Returns a C boolean indicating the current state of the flagType "baseName" flag.
    @return [l_bool]:
     - zero - cleared;
     - non-zero - set. */                                                       \
    static __inline__ l_bool l_flg_tst_ ## flagName(void);                      \
    static __inline__ l_bool l_flg_tst_ ## flagName(void)                       \
    {                                                                           \
        l_bool s = bufName.flagName;                                            \
        return s;                                                               \
    }
```

- *Rule 20.13:*
  - *Violation:* Unrecognized preprocessing directive.
  - *Reasoning:* The static assert macro is used: `ASSERT()`.

Example:
```
ASSERT(sizeof(l_sl1_S2M_3_data_t) == 8);
```

#### MISRA-rigor: advisory
- *Directive 4.8:*
  - *Violation:* This file includes some definitions but does not use it.
- *Directive 4.9:*
  - *Violation:* Function-like macro is used.
  - *Reasoning:* Used to generate the Signals/Flags API
- *Rule 1.2:*
  - *Violation:* The GCC language extensions are used (like `__attribute__`, `__inline__`).
- *Rule 2.5:*
  - *Violation:* Some macro are defined but never used.
  - *Reasoning:* Some macro are shared with constant LIN API.
- *Rule 8.13:*
  - *Violation:* The `const` qualifier can be added to the function parameters as they are not modified in the function scope.
- *Rule 11.4:*
  - *Violation:* The cast of pointer to `uint16_t` is introduced in order to avoid the rule 11.3 violation (required)
- *Rule 13.3:*
  - *Violation:* Post-increment operator is used.
- *Rule 17.8:*
  - *Violation:* Input parameter is modified in the function scope.
- *Rule 19.2:*
  - *Violation:* The unions are used.
- *Rule 20.10:*
  - *Violation:* The `##` preprocessor operator is used.
  - *Reasoning:* Used to generate the Signals/Flags API.

--------------

## The results related to the **constant LIN API** check with Coverity for MISRA 2012

### Intentional MISRA violations reasoning

**MISRA-rigor: required**
- *Rule 2.2:*
  - *Violation:* Function call `mlu_SAE_DataTransmitted(void)` has no effect and can be removed.
  - *Reasoning:* The default `mlu_SAE_DataTransmitted()` implementation is empty, but it's going to be replaced by the application implementation.
- *Rule 8.5:*
  - *Violation:* Some functions are declared more than once.
  - *Reasoning:* There are local weak declarations for functions to be rewritten by the application.
- *Rule 8.6:*
  - *Violation:* Some functions defined by never used.
  - *Reasoning:* These functions are going to be used by the application.
- *Rule 9.1:*
  - *Violation:* Using uninitialized value `data_bytes[LinFrameDataBuffer[4] - 1U]`.
  - *Reasoning:* The `data_bytes` buffer is initialized by `ld_serial_number_callout(data_bytes)` through the pointer.
- *Rule 11.8:*
  - *Violation:* The type cast of the pointer expression `&L_NULL_SCHEDULE` to type `l_Schedule_t*` removes const qualifier from the pointed to type.
  - *Reasoning:* The `L_NULL_SCHEDULE` is put to the `.rodata` (const area) but it's not the case for all schedule tables. The `L_NULL_SCHEDULE` table is special read only schedule table with zero length.
- *Rule 16.1/16.6:*
  - *Violation:* The switch statement does not have two or more conforming switch clauses.
  - *Reasoning:* For some versions on LIN only one cases is relevant.

#### MISRA-rigor: advisory

- *Directive 4.6:*
  - *Violation:* Using basic numerical type `_Bool` rather than a typedef that includes size and signess information.
- *Directive 4.8:*
  - *Violation:* This file includes some definitions but does not use it.
- *Directive 4.9:*
  - *Violation:* Function-like macro is used.
- *Rule 1.2:*
  - *Violation:* The GCC language extensions are used (like `__attribute__`, `__inline__`).
- *Rule 2.3:*
  - *Violation:* Some types defined by never used.
  - *Reasoning:* These types are going to be used by the LDF-dependent LIN API or application.
- *Rule 2.5:*
  - *Violation:* Some macro are defined but never used.
  - *Reasoning:* These types are going to be used by the LDF-dependent LIN API or application.
- *Rule 8.7:*
  - *Violation:* Some functions have external linkage but are only used in one translation unit.
  - *Reasoning:* These functions are going to be used by the application.
- *Rule 8.9:*
  - *Violation:* The `L_NULL_SCHEDULE` should be defined at block scope.
  - *Reasoning:* This definition is going to be used by the application.
- *Rule 8.13:*
  - *Violation:* The `const` qualifier can be added to the function parameters as they are not modified in the function scope.
- *Rule 10.5:*
  - *Violation:* Some explicit type conversion to an inappropriate essential type is used.
- *Rule 11.5:*
  - *Violation:* Void pointer conversion to an object pointer type.
- *Rule 13.3/17.8:*
  - *Violation:* Post-increment operator is used.
- *Rule 15.5:*
  - *Violation:* Input parameter is modified in the function scope.
- *Rule 19.2:*
  - *Violation:* The unions are used.
- *Rule 20.1:*
  - *Violation:* The `#include` directive preceded by code.
- *Rule 20.10:*
  - *Violation:* The `##`/`#` preprocessor operator is used.