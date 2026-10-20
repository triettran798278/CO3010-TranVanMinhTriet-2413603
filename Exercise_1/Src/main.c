/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 2 - Exercise 5: Digital clock with hour & minute on 4 7-SEGs
 *                   - hour = 15, minute = 8, second = 50
 *                   - updateClockBuffer() formats hour and minute into led_buffer[4]
 *                   - 7-segment display scanned at 1Hz (250ms per digit) via TIM2
 *                   - Time incremented every 1s using HAL_Delay(1000)
 ******************************************************************************
 */
#include "main.h"

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

#define MAX_LED 4
#define DOT_PIN GPIO_PIN_4
#define D1_PIN  GPIO_PIN_5

static volatile int index_led = 0;
int led_buffer[MAX_LED] = {0, 0, 0, 0};

int hour = 15;
int minute = 8;
int second = 50;

static const uint16_t digit_en_pins[MAX_LED] = {
    GPIO_PIN_6, // EN0
    GPIO_PIN_7, // EN1
    GPIO_PIN_8, // EN2
    GPIO_PIN_9  // EN3
};

static const uint8_t seg7_code[10] = {
  0x40, 0x79, 0x24, 0x30, 0x19,
  0x12, 0x02, 0x78, 0x00, 0x10
};

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
  for (uint8_t i = 0; i < MAX_LED; i++)
  {
    HAL_GPIO_WritePin(GPIOA, digit_en_pins[i], GPIO_PIN_SET);
  }

  if (index >= 0 && index < MAX_LED)
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

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    static int timer_counter = 0;
    static int dot_counter = 0;

    // Switch 7SEG every 250ms (25 * 10ms) -> 4 * 250ms = 1000ms (1Hz scanning)
    timer_counter++;
    if (timer_counter >= 25)
    {
      timer_counter = 0;
      update7SEG(index_led);
      index_led = (index_led + 1) % MAX_LED;
    }

    // Blink DOT (PA4) and D1 (PA5) every 1s (100 * 10ms)
    dot_counter++;
    if (dot_counter >= 100)
    {
      dot_counter = 0;
      HAL_GPIO_TogglePin(GPIOA, DOT_PIN);
      HAL_GPIO_TogglePin(GPIOA, D1_PIN);
    }
  }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();

  updateClockBuffer();
  update7SEG(0);
  HAL_TIM_Base_Start_IT(&htim2);

  while (1)
  {
    HAL_Delay(1000);
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

  /* Enable AFIO clock and configure SWJ to free PB3/PB4 */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Default safe level: Turn OFF digit enables (HIGH), DOT (HIGH), LED D1 (HIGH) */
  HAL_GPIO_WritePin(GPIOA, DOT_PIN | D1_PIN | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
  /* Default safe level: Turn OFF 7SEG segments (HIGH for Common Anode) */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_SET);

  /* Configure PA4 (DOT), PA5 (LED-RED), PA6 (EN0), PA7 (EN1), PA8 (EN2), PA9 (EN3) */
  GPIO_InitStruct.Pin = DOT_PIN | D1_PIN | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure PB0..PB6 for 7SEG */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
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
