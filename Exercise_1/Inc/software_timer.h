#ifndef __SOFTWARE_TIMER_H
#define __SOFTWARE_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define TICK_INTERVAL_MS   10
#define SOFT_TIMER_NUM_MAX 10

extern volatile uint8_t timer_flag[SOFT_TIMER_NUM_MAX];

void setTimer(uint8_t timer_index, uint16_t duration_ms);
void timerRun(void);

#ifdef __cplusplus
}
#endif

#endif /* __SOFTWARE_TIMER_H */
