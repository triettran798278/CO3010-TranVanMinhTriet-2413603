/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 2 - Exercise 9: 8x8 LED Matrix displaying character 'A'
 *                   - MATRIX-8X8-RED + ULN2803
 *                   - Column enables ENM0..ENM7 on PA2..PA9 (via ULN2803)
 *                   - Row data ROW0..ROW7 on PB8..PB15
 *                   - updateLEDMatrix(int index) invoked in main() via software timer
 ******************************************************************************
 */
#include "main.h"

TIM_HandleTypeDef htim2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);

/* USER CODE BEGIN 0 */
const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;

// Character 'A' bitmap (8 columns, active-HIGH rows)
uint8_t matrix_buffer[8] = {
  0x00, 0x7E, 0x09, 0x09, 0x09, 0x7E, 0x00, 0x00
};

int timer_matrix_counter = 0;
int timer_matrix_flag = 0;
int TIMER_CYCLE = 10;

void setTimerMatrix(int duration)
{
  timer_matrix_counter = duration / TIMER_CYCLE;
  timer_matrix_flag = 0;
}

void timer_run(void)
{
  if (timer_matrix_counter > 0)
  {
    timer_matrix_counter--;
    if (timer_matrix_counter == 0)
    {
      timer_matrix_flag = 1;
    }
  }
}

void setCol(int col)
{
  // Column enables ENM0..ENM7 on PA2..PA9 (Active HIGH into ULN2803)
  uint32_t all_cols = (0xFFU << 2U);
  uint32_t active_col = (1U << (2 + col));
  GPIOA->BSRR = active_col | ((all_cols & ~active_col) << 16U);
}

void setRow(uint8_t data)
{
  // Rows ROW0..ROW7 on PB8..PB15 (Active HIGH anode)
  uint32_t val = ((uint32_t)data) << 8U;
  uint32_t all_rows = (0xFFU << 8U);
  GPIOB->BSRR = (val & all_rows) | (((~val) & all_rows) << 16U);
}

void updateLEDMatrix(int index)
{
  switch (index)
  {
    case 0:
      setCol(0);
      setRow(matrix_buffer[0]);
      break;
    case 1:
      setCol(1);
      setRow(matrix_buffer[1]);
      break;
    case 2:
      setCol(2);
      setRow(matrix_buffer[2]);
      break;
    case 3:
      setCol(3);
      setRow(matrix_buffer[3]);
      break;
    case 4:
      setCol(4);
      setRow(matrix_buffer[4]);
      break;
    case 5:
      setCol(5);
      setRow(matrix_buffer[5]);
      break;
    case 6:
      setCol(6);
      setRow(matrix_buffer[6]);
      break;
    case 7:
      setCol(7);
      setRow(matrix_buffer[7]);
      break;
    default:
      break;
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    timer_run();
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
  // Scan each column every 20ms (smooth, non-flickering refresh)
  setTimerMatrix(20);
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (timer_matrix_flag == 1)
    {
      setTimerMatrix(20);
      updateLEDMatrix(index_led_matrix);
      index_led_matrix = (index_led_matrix + 1) % MAX_LED_MATRIX;
    }
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

  /* Default level: Turn OFF column enables PA2..PA9 (LOW), turn OFF rows PB8..PB15 (LOW) */
  HAL_GPIO_WritePin(GPIOA, 0xFFFC, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, 0xFFFF, GPIO_PIN_RESET);

  /* Configure PA2..PA9 (Columns ENM0..ENM7) */
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5
                      | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure PB8..PB15 (Rows ROW0..ROW7) */
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11
                      | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
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
