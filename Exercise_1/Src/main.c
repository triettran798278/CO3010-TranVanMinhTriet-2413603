/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 2 - Exercise 10: 8x8 LED Matrix marquee animation
 *                   - Character 'A' shifted to the left continuously
 *                   - MATRIX-8X8-RED + ULN2803
 *                   - Column pins: PA2, PA3, PA10, PA11, PA12, PA13, PA14, PA15 (ULN2803)
 *                   - Row pins: PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15
 *                   - JTAG/SWD disabled via __HAL_AFIO_REMAP_SWJ_DISABLE()
 *                   - 7-segment displays: EN0..EN3 on PA6..PA9, segments PB0..PB6
 *                   - DOT on PA4
 *                   - Software timers:
 *                       Timer 0 (1000ms): Digital clock update (hour/min/sec)
 *                       Timer 1 (250ms): 7-segment display scanning
 *                       Timer 2 (500ms): DOT toggle
 *                       Timer 3 (50ms): Matrix column scanning
 *                       Timer 4 (300ms): Matrix shift left animation
 ******************************************************************************
 */
#include "main.h"

#define MATRIX_ROW_PORT    GPIOB
#define MATRIX_COL_PORT    GPIOA
#define DIGIT_EN_PORT      GPIOA
#define SEG_PORT           GPIOB
#define LED_D1_PIN         GPIO_PIN_5
#define DOT_PIN            GPIO_PIN_4
#define LED_NUM            4
#define TICK_INTERVAL_MS   10
#define SOFT_TIMER_NUM_MAX 10
#define MAX_LED_MATRIX     8

TIM_HandleTypeDef htim2;

/* Row pins PB8 -> PB15 (Anodes) */
static uint16_t matrix_row_pins[8] = {
    GPIO_PIN_8,  // ROW0
    GPIO_PIN_9,  // ROW1
    GPIO_PIN_10, // ROW2
    GPIO_PIN_11, // ROW3
    GPIO_PIN_12, // ROW4
    GPIO_PIN_13, // ROW5
    GPIO_PIN_14, // ROW6
    GPIO_PIN_15  // ROW7
};

/* Column enable pins ENM0 -> ENM7 (ULN2803 inputs on GPIOA) */
static uint16_t matrix_col_pins[8] = {
    GPIO_PIN_2,  // ENM0 -> COL0
    GPIO_PIN_3,  // ENM1 -> COL1
    GPIO_PIN_10, // ENM2 -> COL2
    GPIO_PIN_11, // ENM3 -> COL3
    GPIO_PIN_12, // ENM4 -> COL4
    GPIO_PIN_13, // ENM5 -> COL5
    GPIO_PIN_14, // ENM6 -> COL6
    GPIO_PIN_15  // ENM7 -> COL7
};

/* 7-Segment digit enable pins EN0 -> EN3 on GPIOA */
static uint16_t digit_en_pins[4] = {
    GPIO_PIN_6, // EN0
    GPIO_PIN_7, // EN1
    GPIO_PIN_8, // EN2
    GPIO_PIN_9  // EN3
};

/* Digits to display on the 4 7-segment displays */
static uint8_t led_buffer[4] = {1, 2, 3, 0};

/* 8x8 LED Matrix Character 'A' Bitmap */
static uint8_t matrix_bitmap[8] = {
    0x00, // Col 0: 00000000
    0xFC, // Col 1: 11111100
    0x12, // Col 2: 00010010
    0x11, // Col 3: 00010001
    0x11, // Col 4: 00010001
    0x12, // Col 5: 00010010
    0xFC, // Col 6: 11111100
    0x00  // Col 7: 00000000
};

static volatile uint8_t current_matrix_col = 0;
static volatile uint8_t current_seg7_digit = 0;
static volatile uint8_t timer_flag[SOFT_TIMER_NUM_MAX] = {0};
static volatile uint16_t timer_counter[SOFT_TIMER_NUM_MAX] = {0};

int hour = 15, minute = 8, second = 50;

/* 7-Segment Common Anode code (0 = ON, 1 = OFF) */
static const uint8_t seg7_code[10] = {
  0x40, 0x79, 0x24, 0x30, 0x19,
  0x12, 0x02, 0x78, 0x00, 0x10
};

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

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

void display7SEG(int num)
{
  if (num < 0 || num > 9) return;
  uint8_t pattern = seg7_code[num];
  for (int i = 0; i < 7; i++)
  {
    HAL_GPIO_WritePin(GPIOB, (uint16_t)(1U << i),
                      (pattern & (1U << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

void update7SEG(int index)
{
  for (uint8_t i = 0; i < LED_NUM; i++)
  {
    HAL_GPIO_WritePin(GPIOA, digit_en_pins[i], GPIO_PIN_SET);
  }

  if (index >= 0 && index < LED_NUM)
  {
    HAL_GPIO_WritePin(GPIOA, digit_en_pins[index], GPIO_PIN_RESET);
    display7SEG(led_buffer[index]);
  }
}

void updateClockBuffer(void)
{
  led_buffer[0] = hour / 10;
  led_buffer[1] = hour % 10;
  led_buffer[2] = minute / 10;
  led_buffer[3] = minute % 10;
}

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

  // Turn off all columns and rows first
  clearMatrix();

  // Enable active column: RESET allows pull-up resistor to pull column up to +3.3V
  HAL_GPIO_WritePin(MATRIX_COL_PORT, matrix_col_pins[col_index], GPIO_PIN_RESET);

  // Set rows: bit=1 -> RESET pulls cathode to GND -> LED ON; bit=0 -> SET (+3.3V) -> LED OFF
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

void shiftLeft(void)
{
  uint8_t tmp = matrix_bitmap[0];
  for (uint8_t i = 0; i < MAX_LED_MATRIX - 1; i++)
  {
    matrix_bitmap[i] = matrix_bitmap[i + 1];
  }
  matrix_bitmap[MAX_LED_MATRIX - 1] = tmp;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    timerRun();
  }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();

  for (uint8_t i = 0; i < SOFT_TIMER_NUM_MAX; i++)
  {
    setTimer(i, 0);
  }
  clearMatrix();

  HAL_TIM_Base_Start_IT(&htim2);

  setTimer(0, 1000); // Timer 0: 1 second clock increment
  setTimer(1, 250);  // Timer 1: 250 ms 7-seg scanning
  setTimer(2, 500);  // Timer 2: 500 ms DOT toggle
  setTimer(3, 50);   // Timer 3: 50 ms matrix column scanning
  setTimer(4, 300);  // Timer 4: 300 ms matrix shift left animation

  while (1)
  {
    // Timer 0: 1 second clock increment
    if (timer_flag[0])
    {
      timer_flag[0] = 0;
      setTimer(0, 1000);
      second++;
      if (second >= 60)
      {
        second = 0;
        minute++;
        if (minute >= 60)
        {
          minute = 0;
          hour++;
          if (hour >= 24)
          {
            hour = 0;
          }
        }
      }
      updateClockBuffer();
    }

    // Timer 1: 7-segment scanning
    if (timer_flag[1])
    {
      timer_flag[1] = 0;
      setTimer(1, 250);
      update7SEG(current_seg7_digit);
      current_seg7_digit = (current_seg7_digit + 1) % LED_NUM;
    }

    // Timer 2: Toggle DOT & D1
    if (timer_flag[2])
    {
      timer_flag[2] = 0;
      setTimer(2, 500);
      HAL_GPIO_TogglePin(GPIOA, DOT_PIN);
      HAL_GPIO_TogglePin(GPIOA, LED_D1_PIN);
    }

    // Timer 3: 8x8 LED Matrix scanning
    if (timer_flag[3])
    {
      timer_flag[3] = 0;
      setTimer(3, 50);
      updateLedMatrix(current_matrix_col);
      current_matrix_col = (current_matrix_col + 1) % MAX_LED_MATRIX;
    }

    // Timer 4: Shift character 'A' to the left
    if (timer_flag[4])
    {
      timer_flag[4] = 0;
      setTimer(4, 300);
      shiftLeft();
    }
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Enable AFIO clock and disable JTAG/SWD to release PA13, PA14, PA15, PB3, PB4 */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_DISABLE();

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Safe default pin states */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5
                         | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9
                         | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
                         | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_SET);

  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_All, GPIO_PIN_SET);

  /* Configure GPIO pins for GPIOA */
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5
                      | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9
                      | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13
                      | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure GPIO pins for GPIOB */
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
