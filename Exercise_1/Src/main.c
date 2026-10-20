/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 1 - Exercise 5: Integrate 7SEG countdown with 4-way
 *                   traffic light (RED 5s, GREEN 3s, YELLOW 2s).
 *                   Traffic LEDs (PA5..PA10): Active-HIGH (Pin -> 330R -> LED -> GND)
 *                   7-Segment (PB0..PB6): Common Anode / Active-LOW
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

#define WAY1_MASK   (RED1_Pin | YELLOW1_Pin | GREEN1_Pin)
#define WAY2_MASK   (RED2_Pin | YELLOW2_Pin | GREEN2_Pin)
#define ALL_TRAFFIC_MASK (WAY1_MASK | WAY2_MASK)

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

void setTrafficLights(GPIO_PinState r1, GPIO_PinState y1, GPIO_PinState g1,
                      GPIO_PinState r2, GPIO_PinState y2, GPIO_PinState g2)
{
  uint32_t set_mask = 0;
  if (r1 == GPIO_PIN_SET) set_mask |= RED1_Pin;
  if (y1 == GPIO_PIN_SET) set_mask |= YELLOW1_Pin;
  if (g1 == GPIO_PIN_SET) set_mask |= GREEN1_Pin;
  if (r2 == GPIO_PIN_SET) set_mask |= RED2_Pin;
  if (y2 == GPIO_PIN_SET) set_mask |= YELLOW2_Pin;
  if (g2 == GPIO_PIN_SET) set_mask |= GREEN2_Pin;

  uint32_t reset_mask = (~set_mask) & ALL_TRAFFIC_MASK;
  GPIOA->BSRR = set_mask | (reset_mask << 16U);
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  /* USER CODE BEGIN 2 */
  int tick = 0; // 0..9 seconds cycle
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (tick < 3) {
      // Way 1 RED (countdown 5..3), Way 2 GREEN (3..1) [Active-HIGH: SET = ON, RESET = OFF]
      setTrafficLights(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET,
                       GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
      display7SEG(5 - tick);
    } else if (tick < 5) {
      // Way 1 RED (countdown 2..1), Way 2 YELLOW (2..1)
      setTrafficLights(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET,
                       GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
      display7SEG(5 - tick);
    } else if (tick < 8) {
      // Way 1 GREEN (countdown 3..1), Way 2 RED (5..3)
      setTrafficLights(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET,
                       GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
      display7SEG(8 - tick);
    } else {
      // Way 1 YELLOW (countdown 2..1), Way 2 RED (2..1)
      setTrafficLights(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET,
                       GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
      display7SEG(10 - tick);
    }

    HAL_Delay(1000);
    tick = (tick + 1) % 10;
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

  /*Configure GPIO pins : PA5..PA10 */
  GPIO_InitStruct.Pin = ALL_TRAFFIC_MASK;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0..PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                      | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
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
