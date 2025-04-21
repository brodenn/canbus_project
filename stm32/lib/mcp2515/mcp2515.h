#ifndef MCP2515_H
#define MCP2515_H

#include "stm32f4xx_hal.h"

// === MCP2515 command set ===
#define MCP_RESET         0xC0
#define MCP_READ          0x03
#define MCP_WRITE         0x02
#define MCP_BITMOD        0x05
#define MCP_READ_STATUS   0xA0
#define MCP_RTS_TX0       0x81

// === Register addresses ===
// Control & status
#define MCP_CANCTRL       0x0F
#define MCP_CANSTAT       0x0E

// Bit timing
#define MCP_CNF1          0x2A
#define MCP_CNF2          0x29
#define MCP_CNF3          0x28

// TX buffer 0
#define MCP_TXB0CTRL      0x30
#define MCP_TXB0SIDH      0x31
#define MCP_TXB0SIDL      0x32
#define MCP_TXB0EID8      0x33
#define MCP_TXB0EID0      0x34
#define MCP_TXB0DLC       0x35
#define MCP_TXB0D0        0x36
#define MCP_TXB0D1        0x37
#define MCP_TXB0D2        0x38
#define MCP_TXB0D3        0x39
#define MCP_TXB0D4        0x3A
#define MCP_TXB0D5        0x3B
#define MCP_TXB0D6        0x3C
#define MCP_TXB0D7        0x3D

// CANCTRL modes
#define MODE_NORMAL       0x00
#define MODE_SLEEP        0x20
#define MODE_LOOPBACK     0x40
#define MODE_LISTENONLY   0x60
#define MODE_CONFIG       0x80

// === Function declarations ===
void mcp2515_reset(SPI_HandleTypeDef* hspi);
void mcp2515_write_register(SPI_HandleTypeDef* hspi, uint8_t reg, uint8_t value);
uint8_t mcp2515_read_register(SPI_HandleTypeDef* hspi, uint8_t reg);
void mcp2515_bit_modify(SPI_HandleTypeDef* hspi, uint8_t reg, uint8_t mask, uint8_t data);
uint8_t mcp2515_read_status(SPI_HandleTypeDef* hspi);
void mcp2515_set_mode(SPI_HandleTypeDef* hspi, uint8_t mode);
void mcp2515_init(SPI_HandleTypeDef* hspi);
void mcp2515_send_test_frame(SPI_HandleTypeDef* hspi);

#endif // MCP2515_H
