#include "stm32f4xx_hal.h"

// Declare the SPI handle as extern to use in this file
extern SPI_HandleTypeDef hspi1;

void MX_SPI1_Init(void) {
    // Enable SPI1 and GPIOA and GPIOB clocks
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // SPI pins on CN10 (PA5 -> SCK, PA6 -> MISO, PA7 -> MOSI)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;  // SCK, MISO, MOSI
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // MCP2515 Chip Select pin on CN9 (PB6)
    GPIO_InitStruct.Pin = GPIO_PIN_6;  // Chip Select pin
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);  // Set Chip Select high (inactive)

    // SPI configuration
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;  // Set to Master mode
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;  // Full-duplex communication
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;  // 8-bit data frames
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;  // Clock polarity low
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;  // First edge is valid
    hspi1.Init.NSS = SPI_NSS_SOFT;  // Software-controlled chip select
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  // Baud rate prescaler
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;  // Most significant bit first
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;  // No TI mode
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;  // Disable CRC calculation
    hspi1.Init.CRCPolynomial = 10;  // Polynomial for CRC (not used)
    HAL_SPI_Init(&hspi1);  // Initialize the SPI peripheral
}
