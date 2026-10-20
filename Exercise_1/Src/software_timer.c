#include "software_timer.h"

volatile uint8_t timer_flag[SOFT_TIMER_NUM_MAX] = {0};
static volatile uint16_t timer_counter[SOFT_TIMER_NUM_MAX] = {0};

void setTimer(uint8_t timer_index, uint16_t duration_ms)
{
  if (timer_index < SOFT_TIMER_NUM_MAX)
  {
    timer_counter[timer_index] = duration_ms / TICK_INTERVAL_MS;
    timer_flag[timer_index] = 0;
  }
}

void timerRun(void)
{
  for (uint8_t i = 0; i < SOFT_TIMER_NUM_MAX; i++)
  {
    if (timer_counter[i] > 0)
    {
      timer_counter[i]--;
      if (timer_counter[i] == 0)
      {
        timer_flag[i] = 1;
      }
    }
  }
}
