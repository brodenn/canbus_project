#include "stm32f4xx_hal.h"

void MX_GPIO_Init(void) {
    // Enable GPIOA and GPIOC clock
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // === PA5: Output pin for LED ===
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // === PC13: Input pin for Button B1 ===
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;

    // NUCLEO-F446RE uses a **blue user button** (B1) on **PC13** which is connected to GND when pressed
    // and is pulled up by a weak pull-up resistor. Therefore, no internal pull-up/down needed.
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}
