#ifndef SWTIMER_CONFIG_H
#define SWTIMER_CONFIG_H

#define CT_PERIODIC_RATE (500U) /* us */
#define CT_INT_PRIORITY (6u) /* 3-6 */

typedef enum swtimer_id
{
    SWTIMER_1,
    SWTIMER_2,
    SWTIMER_3,
    SWTIMER_4,
    SWTIMER_NR_OF
} swtimer_id_t;

#endif /* SWTIMER_CONFIG_H */
/* EOF */
