#include "seg7.h"

int hour = 15, minute = 8, second = 50;
uint8_t led_buffer[LED_NUM] = {1, 2, 3, 0};

static const uint16_t digit_en_pins[LED_NUM] = {
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_8,
    GPIO_PIN_9
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
