/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 1 - Exercise 5: Integrate 7SEG countdown with 4-way
 *                   traffic light (RED 5s, GREEN 3s, YELLOW 2s).
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
  int tick = 0; // 0..9 seconds cycle
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (tick < 3) {
      // Way 1 RED (countdown 5..3), Way 2 GREEN (3..1)
      setTrafficWay1(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET);
      setTrafficWay2(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
      display7SEG(5 - tick);
    } else if (tick < 5) {
      // Way 1 RED (countdown 2..1), Way 2 YELLOW (2..1)
      setTrafficWay1(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET);
      setTrafficWay2(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
      display7SEG(5 - tick);
    } else if (tick < 8) {
      // Way 1 GREEN (countdown 3..1), Way 2 RED (5..3)
      setTrafficWay1(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
      setTrafficWay2(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET);
      display7SEG(8 - tick);
    } else {
      // Way 1 YELLOW (countdown 2..1), Way 2 RED (2..1)
      setTrafficWay1(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
      setTrafficWay2(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET);
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

  /*Configure GPIO pin Output Level: Default HIGH (LEDs OFF for active-low) */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_All, GPIO_PIN_SET);
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

