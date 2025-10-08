#include "stm32f4xx_hal.h"

int main(void)
{
    HAL_Init();                         // cần cho HAL (NVIC, SysTick init)

    __HAL_RCC_GPIOD_CLK_ENABLE();       // bật clock Port D

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.Pin   = GPIO_PIN_12;
    GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull  = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

    HAL_GPIO_WritePin(GPIOD, GPIO_InitStructure.Pin, GPIO_PIN_SET);  // bật sáng 4 LED

    while (1) { /* giữ sáng */ }
}
