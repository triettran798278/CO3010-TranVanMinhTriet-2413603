/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 1 - Exercise 3: 4-way traffic light
 *                   Way 1 (Vertical): PA5 (RED1), PA6 (YELLOW1), PA7 (GREEN1)
 *                   Way 2 (Horizontal): PA8 (RED2), PA9 (YELLOW2), PA10 (GREEN2)
 *                   Wiring: Pin -> 330 Ohm -> LED -> GND (Active-HIGH: SET = ON, RESET = OFF)
 *                   using a single HAL_Delay(1000) at the end of while(1).
 ******************************************************************************
 */
#include "main.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* USER CODE BEGIN 0 */
#define RED1_Pin    GPIO_PIN_5
#define YELLOW1_Pin GPIO_PIN_6
#define GREEN1_Pin  GPIO_PIN_7

#define RED2_Pin    GPIO_PIN_8
#define YELLOW2_Pin GPIO_PIN_9
#define GREEN2_Pin  GPIO_PIN_10

void setTrafficWay1(GPIO_PinState r, GPIO_PinState y, GPIO_PinState g) {
  HAL_GPIO_WritePin(GPIOA, RED1_Pin, r);
  HAL_GPIO_WritePin(GPIOA, YELLOW1_Pin, y);
  HAL_GPIO_WritePin(GPIOA, GREEN1_Pin, g);
}

void setTrafficWay2(GPIO_PinState r, GPIO_PinState y, GPIO_PinState g) {
  HAL_GPIO_WritePin(GPIOA, RED2_Pin, r);
  HAL_GPIO_WritePin(GPIOA, YELLOW2_Pin, y);
  HAL_GPIO_WritePin(GPIOA, GREEN2_Pin, g);
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  int counter = 0; // 0..9 seconds cycle
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (counter < 3)
    {
      // Phase 1 (3s): Way 1 RED (5s total), Way 2 GREEN (3s) [Active-HIGH: SET = ON, RESET = OFF]
      setTrafficWay1(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
      setTrafficWay2(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
    }
    else if (counter < 5)
    {
      // Phase 2 (2s): Way 1 still RED, Way 2 YELLOW (2s)
      setTrafficWay1(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
      setTrafficWay2(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
    }
    else if (counter < 8)
    {
      // Phase 3 (3s): Way 1 GREEN (3s), Way 2 RED (5s total)
      setTrafficWay1(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
      setTrafficWay2(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
    }
    else
    {
      // Phase 4 (2s): Way 1 YELLOW (2s), Way 2 still RED
      setTrafficWay1(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
      setTrafficWay2(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
    }

    counter++;
    if (counter >= 10) counter = 0;

    HAL_Delay(1000);
    /* USER CODE END WHILE */
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

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NOJTAG();

  /*Configure GPIO pin Output Level: GPIOA LOW (Active-HIGH LEDs OFF), GPIOB HIGH (Common-Anode 7-seg OFF) */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_All, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_All, GPIO_PIN_SET);

  /*Configure GPIO pins : PA0..PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_All;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0..PB15 */
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
