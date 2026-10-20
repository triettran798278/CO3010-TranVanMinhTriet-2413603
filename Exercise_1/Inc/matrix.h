#ifndef __MATRIX_H
#define __MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define MAX_LED_MATRIX 8

extern uint8_t matrix_bitmap[MAX_LED_MATRIX];

void clearMatrix(void);
void updateMatrixRows(uint8_t col_index);
void updateLedMatrix(uint8_t index);

#ifdef __cplusplus
}
#endif

#endif /* __MATRIX_H */
