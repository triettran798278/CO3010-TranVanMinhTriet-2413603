#include "matrix.h"

#define MATRIX_ROW_PORT GPIOB
#define MATRIX_COL_PORT GPIOA

static const uint16_t matrix_row_pins[MAX_LED_MATRIX] = {
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15
};

static const uint16_t matrix_col_pins[MAX_LED_MATRIX] = {
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_10,
    GPIO_PIN_11,
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15
};

uint8_t matrix_bitmap[MAX_LED_MATRIX] = {
    0x00, 0xFC, 0x12, 0x11, 0x11, 0x12, 0xFC, 0x00
};

static const uint8_t matrix_scroll_data[MATRIX_SCROLL_WIDTH] = {
    0x00, 0xFC, 0x12, 0x11, 0x11, 0x12, 0xFC, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static int matrix_scroll_position = 0;

void clearMatrix(void)
{
  for (uint8_t i = 0; i < MAX_LED_MATRIX; i++)
  {
    HAL_GPIO_WritePin(MATRIX_COL_PORT, matrix_col_pins[i], GPIO_PIN_SET);
  }
  for (uint8_t row = 0; row < MAX_LED_MATRIX; row++)
  {
    HAL_GPIO_WritePin(MATRIX_ROW_PORT, matrix_row_pins[row], GPIO_PIN_SET);
  }
}

void updateMatrixRows(uint8_t col_index)
{
  if (col_index >= MAX_LED_MATRIX)
    return;

  clearMatrix();

  HAL_GPIO_WritePin(MATRIX_COL_PORT, matrix_col_pins[col_index], GPIO_PIN_RESET);

  uint8_t row_data = matrix_bitmap[col_index];
  for (uint8_t row = 0; row < MAX_LED_MATRIX; row++)
  {
    GPIO_PinState pin_state = (row_data & (1U << row)) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin(MATRIX_ROW_PORT, matrix_row_pins[row], pin_state);
  }
}

void updateLedMatrix(uint8_t index)
{
  switch (index)
  {
    case 0: updateMatrixRows(0); break;
    case 1: updateMatrixRows(1); break;
    case 2: updateMatrixRows(2); break;
    case 3: updateMatrixRows(3); break;
    case 4: updateMatrixRows(4); break;
    case 5: updateMatrixRows(5); break;
    case 6: updateMatrixRows(6); break;
    case 7: updateMatrixRows(7); break;
    default: break;
  }
}

void scrollMatrixLeft(void)
{
  matrix_scroll_position = (matrix_scroll_position + 1) % MATRIX_SCROLL_WIDTH;
  for (int col = 0; col < MAX_LED_MATRIX; col++)
  {
    int data_index = (col + matrix_scroll_position) % MATRIX_SCROLL_WIDTH;
    matrix_bitmap[col] = matrix_scroll_data[data_index];
  }
}
