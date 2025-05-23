#include "stm32f4xx_hal.h"
#include "mcp2515.h"
#include <string.h>
#include <stdio.h>

#define MCP_RXB1CTRL      0x70
#define MCP_RXB1SIDH      0x71
#define MCP_RXB1SIDL      0x72
#define MCP_RXB1DLC       0x75
#define MCP_RXB1D0        0x76

#define CAN_RX_BUFFER_SIZE 32

typedef struct {
    uint16_t id;
    uint8_t dlc;
    uint8_t data[8];
} CanRxFrame;

volatile CanRxFrame can_rx_buffer[CAN_RX_BUFFER_SIZE];
volatile uint8_t can_rx_head = 0;
volatile uint8_t can_rx_tail = 0;

UART_HandleTypeDef huart2;
SPI_HandleTypeDef hspi1;

void SystemClock_Config(void);
void MX_USART2_UART_Init(void);
void MX_GPIO_Init(void);
void MX_SPI1_Init(void);

void uart_print(const char* msg) {
    HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

void byte_to_hex(uint8_t byte, char* str) {
    const char hex[] = "0123456789ABCDEF";
    str[0] = hex[(byte >> 4) & 0x0F];
    str[1] = hex[byte & 0x0F];
    str[2] = '\0';
}

void SysTick_Handler(void) {
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

void buffer_rx_frame(uint8_t sidh, uint8_t sidl, uint8_t dlc, uint8_t base_reg) {
    uint16_t id = ((sidh << 3) | (sidl >> 5)) & 0x7FF;
    if (dlc == 0 || dlc > 8) return;

    uint8_t next = (can_rx_head + 1) % CAN_RX_BUFFER_SIZE;
    if (next == can_rx_tail) return; // Buffer full

    CanRxFrame* frame = &can_rx_buffer[can_rx_head];
    frame->id = id;
    frame->dlc = dlc;

    for (uint8_t i = 0; i < dlc; i++) {
        frame->data[i] = mcp2515_read_register(&hspi1, base_reg + i);
    }

    can_rx_head = next;
}

void print_rx_frame(const CanRxFrame* frame) {
    char buf[64];
    sprintf(buf, "RX Buffered: ID=0x%03X DLC=%d Data: ", frame->id, frame->dlc);
    uart_print(buf);

    for (uint8_t i = 0; i < frame->dlc; i++) {
        char hex[4];
        byte_to_hex(frame->data[i], hex);
        uart_print(hex);
        uart_print(" ");
    }
    uart_print("\r\n");
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_SPI1_Init();

    HAL_Delay(500);
    uart_print("MCP2515 CAN TX/RX Setup...\r\n");

    mcp2515_init(&hspi1);
    mcp2515_write_register(&hspi1, MCP_RXB0CTRL, 0x60);
    mcp2515_write_register(&hspi1, MCP_RXB1CTRL, 0x60);
    mcp2515_set_mode(&hspi1, MODE_NORMAL);
    uart_print("MCP2515 ready in NORMAL mode.\r\n");

    uint8_t counter = 0;
    uint32_t last_send_time = 0;

    while (1) {
        if (HAL_GetTick() - last_send_time >= 1000) {
            uint8_t tx_data[4] = { 0x55, 0xAA, 0x00, counter++ };
            mcp2515_send_message(&hspi1, 0x321, tx_data, 4);
            uart_print("Sent: ID=0x321 Data: ");
            for (int i = 0; i < 4; i++) {
                char hex[3];
                byte_to_hex(tx_data[i], hex);
                uart_print(hex);
                uart_print(" ");
            }
            uart_print("\r\n");
            last_send_time = HAL_GetTick();
        }

        uint8_t status = mcp2515_read_status(&hspi1);

        if (status & 0x01) {
            buffer_rx_frame(
                mcp2515_read_register(&hspi1, MCP_RXB0SIDH),
                mcp2515_read_register(&hspi1, MCP_RXB0SIDL),
                mcp2515_read_register(&hspi1, MCP_RXB0DLC) & 0x0F,
                MCP_RXB0D0
            );
            mcp2515_bit_modify(&hspi1, MCP_CANINTF, 0x01, 0x00);
        }

        if (status & 0x02) {
            buffer_rx_frame(
                mcp2515_read_register(&hspi1, MCP_RXB1SIDH),
                mcp2515_read_register(&hspi1, MCP_RXB1SIDL),
                mcp2515_read_register(&hspi1, MCP_RXB1DLC) & 0x0F,
                MCP_RXB1D0
            );
            mcp2515_bit_modify(&hspi1, MCP_CANINTF, 0x02, 0x00);
        }

        while (can_rx_tail != can_rx_head) {
            print_rx_frame(&can_rx_buffer[can_rx_tail]);
            can_rx_tail = (can_rx_tail + 1) % CAN_RX_BUFFER_SIZE;
        }

        HAL_Delay(1);
    }
}