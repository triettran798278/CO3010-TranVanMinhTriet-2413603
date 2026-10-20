/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Lab 2 - Exercise 9: 8x8 LED Matrix displaying character 'A'
 *                   - Based directly on main_lab2_ex10.c (static 'A')
 *                   - MATRIX-8X8-RED + ULN2803
 *                   - Column pins: PA2, PA3, PA10..PA15 (ENM0..ENM7)
 *                   - Row pins: PB8..PB15 (ROW0..ROW7)
 *                   - 7-Segment display: EN0..EN3 on PA6..PA9, SEG0..SEG6 on PB0..PB6
 *                   - DOT on PA4
 *                   - Software timers (TIM2 10ms interrupt):
 *                       Timer 0 (1000ms): Clock timekeeping (hour/min/sec)
 *                       Timer 1 (1000ms): DOT blinking
 *                       Timer 2 (250ms): 7-segment display scanning
 *                       Timer 3 (10ms): Matrix column scanning
 ******************************************************************************
 */
#include "main.h"

#define SEG_ON  GPIO_PIN_RESET
#define SEG_OFF GPIO_PIN_SET

#define ENM0_Pin GPIO_PIN_2
#define ENM1_Pin GPIO_PIN_3
#define DOT_Pin  GPIO_PIN_4
#define EN0_Pin  GPIO_PIN_6
#define EN1_Pin  GPIO_PIN_7
#define EN2_Pin  GPIO_PIN_8
#define EN3_Pin  GPIO_PIN_9
#define ENM2_Pin GPIO_PIN_10
#define ENM3_Pin GPIO_PIN_11
#define ENM4_Pin GPIO_PIN_12
#define ENM5_Pin GPIO_PIN_13
#define ENM6_Pin GPIO_PIN_14
#define ENM7_Pin GPIO_PIN_15

#define SEG0_Pin GPIO_PIN_0
#define SEG1_Pin GPIO_PIN_1
#define SEG2_Pin GPIO_PIN_2
#define SEG3_Pin GPIO_PIN_3
#define SEG4_Pin GPIO_PIN_4
#define SEG5_Pin GPIO_PIN_5
#define SEG6_Pin GPIO_PIN_6
#define ROW0_Pin GPIO_PIN_8
#define ROW1_Pin GPIO_PIN_9
#define ROW2_Pin GPIO_PIN_10
#define ROW3_Pin GPIO_PIN_11
#define ROW4_Pin GPIO_PIN_12
#define ROW5_Pin GPIO_PIN_13
#define ROW6_Pin GPIO_PIN_14
#define ROW7_Pin GPIO_PIN_15

TIM_HandleTypeDef htim2;

volatile int timer0_counter = 0;
volatile int timer0_flag = 0;
volatile int timer1_counter = 0;
volatile int timer1_flag = 0;
volatile int timer2_counter = 0;
volatile int timer2_flag = 0;
volatile int timer3_counter = 0;
volatile int timer3_flag = 0;
const int TIMER_CYCLE = 10;

const int MAX_LED = 4;
int index_led = 0;

int led_buffer[4] = {0, 0, 0, 0};
int hour = 15, minute = 8, second = 50;
const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;

uint8_t matrix_buffer[8] = {
    0x00, 0x7C, 0x0A, 0x09,
    0x09, 0x0A, 0x7C, 0x00
};

void SystemClock_Config(void);
static void MX_TIM2_Init(void);
static void MX_GPIO_Init(void);

static uint16_t matrix_enable_pin(int index)
{
    switch (index)
    {
        case 0: return ENM0_Pin;
        case 1: return ENM1_Pin;
        case 2: return ENM2_Pin;
        case 3: return ENM3_Pin;
        case 4: return ENM4_Pin;
        case 5: return ENM5_Pin;
        case 6: return ENM6_Pin;
        case 7: return ENM7_Pin;
        default: return 0;
    }
}

static uint16_t matrix_row_pin(int index)
{
    switch (index)
    {
        case 0: return ROW0_Pin;
        case 1: return ROW1_Pin;
        case 2: return ROW2_Pin;
        case 3: return ROW3_Pin;
        case 4: return ROW4_Pin;
        case 5: return ROW5_Pin;
        case 6: return ROW6_Pin;
        case 7: return ROW7_Pin;
        default: return 0;
    }
}

void setTimer0(int duration)
{
    timer0_counter = duration / TIMER_CYCLE;
    timer0_flag = 0;
}

void timer_run(void)
{
    if (timer0_counter > 0)
    {
        timer0_counter--;
        if (timer0_counter == 0)
        {
            timer0_flag = 1;
        }
    }
}

void setTimer1(int duration)
{
    timer1_counter = duration / TIMER_CYCLE;
    timer1_flag = 0;
}

void timer1_run(void)
{
    if (timer1_counter > 0)
    {
        timer1_counter--;
        if (timer1_counter == 0)
        {
            timer1_flag = 1;
        }
    }
}

void setTimer2(int duration)
{
    timer2_counter = duration / TIMER_CYCLE;
    timer2_flag = 0;
}

void timer2_run(void)
{
    if (timer2_counter > 0)
    {
        timer2_counter--;
        if (timer2_counter == 0)
        {
            timer2_flag = 1;
        }
    }
}

void setTimer3(int duration)
{
    timer3_counter = duration / TIMER_CYCLE;
    timer3_flag = 0;
}

void timer3_run(void)
{
    if (timer3_counter > 0)
    {
        timer3_counter--;
        if (timer3_counter == 0)
        {
            timer3_flag = 1;
        }
    }
}

void updateLEDMatrix(int index)
{
    if (index < 0 || index >= MAX_LED_MATRIX)
    {
        return;
    }

    // Turn off all columns first
    for (int col = 0; col < MAX_LED_MATRIX; col++)
    {
        HAL_GPIO_WritePin(GPIOA, matrix_enable_pin(col), GPIO_PIN_RESET);
    }

    // Output row data for the active column
    uint8_t row_data = matrix_buffer[index];
    for (int row = 0; row < MAX_LED_MATRIX; row++)
    {
        HAL_GPIO_WritePin(GPIOB, matrix_row_pin(row),
                          (row_data & (1U << row)) ? GPIO_PIN_SET
                                                   : GPIO_PIN_RESET);
    }

    // Turn on the active column
    HAL_GPIO_WritePin(GPIOA, matrix_enable_pin(index), GPIO_PIN_SET);
}

static void set7SEG(GPIO_PinState a,
                    GPIO_PinState b,
                    GPIO_PinState c,
                    GPIO_PinState d,
                    GPIO_PinState e,
                    GPIO_PinState f,
                    GPIO_PinState g)
{
    HAL_GPIO_WritePin(GPIOB, SEG0_Pin, a);
    HAL_GPIO_WritePin(GPIOB, SEG1_Pin, b);
    HAL_GPIO_WritePin(GPIOB, SEG2_Pin, c);
    HAL_GPIO_WritePin(GPIOB, SEG3_Pin, d);
    HAL_GPIO_WritePin(GPIOB, SEG4_Pin, e);
    HAL_GPIO_WritePin(GPIOB, SEG5_Pin, f);
    HAL_GPIO_WritePin(GPIOB, SEG6_Pin, g);
}

void display7SEG(int num)
{
    switch (num)
    {
        case 0:
            set7SEG(SEG_ON, SEG_ON, SEG_ON, SEG_ON,
                    SEG_ON, SEG_ON, SEG_OFF);
            break;

        case 1:
            set7SEG(SEG_OFF, SEG_ON, SEG_ON, SEG_OFF,
                    SEG_OFF, SEG_OFF, SEG_OFF);
            break;

        case 2:
            set7SEG(SEG_ON, SEG_ON, SEG_OFF, SEG_ON,
                    SEG_ON, SEG_OFF, SEG_ON);
            break;

        case 3:
            set7SEG(SEG_ON, SEG_ON, SEG_ON, SEG_ON,
                    SEG_OFF, SEG_OFF, SEG_ON);
            break;

        case 4:
            set7SEG(SEG_OFF, SEG_ON, SEG_ON, SEG_OFF,
                    SEG_OFF, SEG_ON, SEG_ON);
            break;

        case 5:
            set7SEG(SEG_ON, SEG_OFF, SEG_ON, SEG_ON,
                    SEG_OFF, SEG_ON, SEG_ON);
            break;

        case 6:
            set7SEG(SEG_ON, SEG_OFF, SEG_ON, SEG_ON,
                    SEG_ON, SEG_ON, SEG_ON);
            break;

        case 7:
            set7SEG(SEG_ON, SEG_ON, SEG_ON, SEG_OFF,
                    SEG_OFF, SEG_OFF, SEG_OFF);
            break;

        case 8:
            set7SEG(SEG_ON, SEG_ON, SEG_ON, SEG_ON,
                    SEG_ON, SEG_ON, SEG_ON);
            break;

        case 9:
            set7SEG(SEG_ON, SEG_ON, SEG_ON, SEG_ON,
                    SEG_OFF, SEG_ON, SEG_ON);
            break;

        default:
            set7SEG(SEG_OFF, SEG_OFF, SEG_OFF, SEG_OFF,
                    SEG_OFF, SEG_OFF, SEG_OFF);
            break;
    }
}

void update7SEG(int index)
{
    HAL_GPIO_WritePin(GPIOA, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin, GPIO_PIN_SET);

    switch (index)
    {
        case 0:
            display7SEG(led_buffer[0]);
            HAL_GPIO_WritePin(GPIOA, EN0_Pin, GPIO_PIN_RESET);
            break;

        case 1:
            display7SEG(led_buffer[1]);
            HAL_GPIO_WritePin(GPIOA, EN1_Pin, GPIO_PIN_RESET);
            break;

        case 2:
            display7SEG(led_buffer[2]);
            HAL_GPIO_WritePin(GPIOA, EN2_Pin, GPIO_PIN_RESET);
            break;

        case 3:
            display7SEG(led_buffer[3]);
            HAL_GPIO_WritePin(GPIOA, EN3_Pin, GPIO_PIN_RESET);
            break;

        default:
            break;
    }
}

void updateClockBuffer(void)
{
    led_buffer[0] = hour / 10;
    led_buffer[1] = hour % 10;
    led_buffer[2] = minute / 10;
    led_buffer[3] = minute % 10;
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_TIM2_Init();
  MX_GPIO_Init();

  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_DISABLE();

  // Ban dau tat ca cac cot va hang cua Matrix deu tat
  HAL_GPIO_WritePin(GPIOA,
      ENM0_Pin | ENM1_Pin | ENM2_Pin | ENM3_Pin |
      ENM4_Pin | ENM5_Pin | ENM6_Pin | ENM7_Pin,
      GPIO_PIN_RESET);

  HAL_GPIO_WritePin(GPIOB,
      ROW0_Pin | ROW1_Pin | ROW2_Pin | ROW3_Pin |
      ROW4_Pin | ROW5_Pin | ROW6_Pin | ROW7_Pin,
      GPIO_PIN_RESET);

  updateClockBuffer();
  setTimer0(1000);   // clock
  setTimer1(1000);   // DOT
  setTimer2(250);    // 7-segment scanning
  setTimer3(10);     // LED Matrix: 10ms per column

  HAL_TIM_Base_Start_IT(&htim2);

  while (1)
  {
    if (timer0_flag == 1)
    {
        second++;
        if (second >= 60)
        {
            second = 0;
            minute++;
        }
        if (minute >= 60)
        {
            minute = 0;
            hour++;
        }
        if (hour >= 24)
        {
            hour = 0;
        }
        updateClockBuffer();
        setTimer0(1000);
    }

    if (timer1_flag == 1)
    {
        HAL_GPIO_TogglePin(GPIOA, DOT_Pin);
        setTimer1(1000);
    }

    if (timer2_flag == 1)
    {
        update7SEG(index_led);
        index_led++;
        if (index_led >= MAX_LED)
        {
            index_led = 0;
        }
        setTimer2(250);
    }

    if (timer3_flag == 1)
    {
        updateLEDMatrix(index_led_matrix);
        index_led_matrix++;
        if (index_led_matrix >= MAX_LED_MATRIX)
        {
            index_led_matrix = 0;
        }
        setTimer3(10);
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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|DOT_Pin|EN0_Pin
                          |EN1_Pin|EN2_Pin|EN3_Pin|ENM2_Pin
                          |ENM3_Pin|ENM4_Pin|ENM5_Pin|ENM6_Pin
                          |ENM7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins for GPIOA */
  GPIO_InitStruct.Pin = ENM0_Pin|ENM1_Pin|DOT_Pin|EN0_Pin
                          |EN1_Pin|EN2_Pin|EN3_Pin|ENM2_Pin
                          |ENM3_Pin|ENM4_Pin|ENM5_Pin|ENM6_Pin
                          |ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins for GPIOB */
  GPIO_InitStruct.Pin = SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timer_run();
        timer1_run();
        timer2_run();
        timer3_run();
    }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
