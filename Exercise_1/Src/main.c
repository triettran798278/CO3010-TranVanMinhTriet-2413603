/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 2 - Exercise 2: 4 seven-segment LEDs & DOT (PA4)
 *                   - 7SEG enables on PA6..PA9 (EN0..EN3, PNP active-low)
 *                   - 7SEG segments on PB0..PB6 (Common Anode)
 *                   - Displays '1', '2', '3', '0' (representing 12:30)
 *                   - Each 7SEG stays on for 500ms (cycle = 2000ms, f = 0.5Hz)
 *                   - DOT (PA4) blinks every second
 ******************************************************************************
 */
#include "main.h"

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN 0 */
static const uint8_t seg7_code[10] = {
  0x40, 0x79, 0x24, 0x30, 0x19,
  0x12, 0x02, 0x78, 0x00, 0x10
};

void display7SEG(int num)
{
  if (num < 0 || num > 9) return;
  uint32_t set_mask = (uint32_t)(seg7_code[num] & 0x7FU);
  uint32_t reset_mask = (~set_mask) & 0x7FU;
  GPIOB->BSRR = set_mask | (reset_mask << 16U);
}

void setEnablePins(int en0, int en1, int en2, int en3)
{
  // PA6=EN0, PA7=EN1, PA8=EN2, PA9=EN3
  // 0 = ON (RESET), 1 = OFF (SET)
  uint32_t set_mask = 0;
  uint32_t reset_mask = 0;

  if (en0) set_mask |= GPIO_PIN_6; else reset_mask |= GPIO_PIN_6;
  if (en1) set_mask |= GPIO_PIN_7; else reset_mask |= GPIO_PIN_7;
  if (en2) set_mask |= GPIO_PIN_8; else reset_mask |= GPIO_PIN_8;
  if (en3) set_mask |= GPIO_PIN_9; else reset_mask |= GPIO_PIN_9;

  GPIOA->BSRR = set_mask | (reset_mask << 16U);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    static int tick = 0;
    tick++;
    if (tick >= 200) tick = 0; // 200 * 10ms = 2000ms = 2s

    // 4 LEDs display "1 2 3 0" (500ms each)
    if (tick < 50)
    {
      setEnablePins(0, 1, 1, 1);
      display7SEG(1);
    }
    else if (tick < 100)
    {
      setEnablePins(1, 0, 1, 1);
      display7SEG(2);
    }
    else if (tick < 150)
    {
      setEnablePins(1, 1, 0, 1);
      display7SEG(3);
    }
    else
    {
      setEnablePins(1, 1, 1, 0);
      display7SEG(0);
    }

    // Blink DOT (PA4) every 1s (toggle every 100 ticks = 1000ms)
    if (tick % 100 == 0)
    {
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
      HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
  }
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* Everything handled in Timer interrupt */
  }
  /* USER CODE END WHILE */
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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /* Default level: Turn OFF PNP enables (HIGH), DOT (HIGH), LED-RED (HIGH) */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_SET);
  /* Default level: Turn OFF 7SEG segments (HIGH for Common Anode) */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_SET);

  /* Configure PA4 (DOT), PA5 (LED-RED), PA6 (EN0), PA7 (EN1), PA8 (EN2), PA9 (EN3) */
  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
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
