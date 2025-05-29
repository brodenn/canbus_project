#include "stm32f4xx_hal.h"
#include "mcp2515.h"
#include "can_buffer.h"
#include "uart_log.h"
#include "utils.h"

extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART2_UART_Init(void);
void MX_SPI1_Init(void);

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

    uint8_t counter = 0, toggle = 0;
    float temperature = 22.0, battery_voltage = 11.8;
    uint32_t last_send_time = 0, last_crash_trigger = 0;
    uint8_t button_sent = 0, b1_state = 0;

    while (1) {
        uint32_t now = HAL_GetTick();

        if (now - last_send_time >= 1000) {
            uint8_t self_test[4] = { 0x55, 0xAA, 0x00, counter++ };
            mcp2515_send_message(&hspi1, 0x321, self_test, 4);
            uart_print("Sent 0x321: STM32 Test\r\n");

            uint8_t high_beam[1] = { toggle };
            mcp2515_send_message(&hspi1, 0x110, high_beam, 1);
            uart_print(toggle ? "Sent 0x110: High Beam ON\r\n" : "Sent 0x110: High Beam OFF\r\n");

            float voltage = battery_voltage + ((rand() % 5) - 2) * 0.05f;
            if (voltage < 11.4) voltage = 11.4;
            if (voltage > 12.6) voltage = 12.6;
            uint16_t voltage_fixed = (uint16_t)(voltage * 100);
            uint8_t battery_alert[2] = { (voltage_fixed >> 8) & 0xFF, voltage_fixed & 0xFF };
            mcp2515_send_message(&hspi1, 0x120, battery_alert, 2);
            char voltbuf[32];
            sprintf(voltbuf, "Sent 0x120: Battery %.2fV\r\n", voltage);
            uart_print(voltbuf);

            if ((rand() % 10) == 0 || (now - last_crash_trigger > 10000)) {
                uint8_t crash_trigger[2] = { 0xDE, 0xAD };
                mcp2515_send_message(&hspi1, 0x130, crash_trigger, 2);
                uart_print("Sent 0x130: Crash Trigger \xF0\x9F\x94\xB4\r\n");
                last_crash_trigger = now;
            }

            temperature += 0.2f;
            if (temperature > 28.0f) temperature = 22.0f;
            uint16_t temp_fixed = (uint16_t)(temperature * 256);
            uint8_t temp_data[2] = { (temp_fixed >> 8) & 0xFF, temp_fixed & 0xFF };
            mcp2515_send_message(&hspi1, 0x140, temp_data, 2);
            char tmpbuf[32];
            sprintf(tmpbuf, "Sent 0x140: Temperature %.1f\xC2\xB0C\r\n", temperature);
            uart_print(tmpbuf);

            uint8_t blinker[1] = { toggle };
            mcp2515_send_message(&hspi1, 0x150, blinker, 1);
            uart_print(toggle ? "Sent 0x150: Blinker RIGHT\r\n" : "Sent 0x150: Blinker LEFT\r\n");

            toggle ^= 1;
            last_send_time = now;
        }

        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) {
            if (!button_sent) {
                b1_state ^= 1;
                uint8_t button_msg[1] = { b1_state };
                mcp2515_send_message(&hspi1, 0x160, button_msg, 1);
                uart_print(b1_state ? "Sent 0x160: B1 PRESSED\r\n" : "Sent 0x160: B1 RELEASED\r\n");
                button_sent = 1;
            }
        } else {
            button_sent = 0;
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
            CanRxFrame* f = &can_rx_buffer[can_rx_tail];
            print_rx_frame(f);
            handle_rx_frame(f);
            can_rx_tail = (can_rx_tail + 1) % CAN_RX_BUFFER_SIZE;
        }

        HAL_Delay(1);
    }
}
