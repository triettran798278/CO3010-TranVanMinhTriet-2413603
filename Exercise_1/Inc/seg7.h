#ifndef __SEG7_H
#define __SEG7_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define LED_NUM 4

extern int hour;
extern int minute;
extern int second;
extern uint8_t led_buffer[LED_NUM];

void display7SEG(int num);
void update7SEG(int index);
void updateClockBuffer(void);

#ifdef __cplusplus
}
#endif

#endif /* __SEG7_H */
