#include "mcp2515.h"

#define MCP_CS_PORT GPIOB
#define MCP_CS_PIN  GPIO_PIN_6

// --- SPI Chip Select ---
static void mcp_select() {
    HAL_GPIO_WritePin(MCP_CS_PORT, MCP_CS_PIN, GPIO_PIN_RESET);
}

static void mcp_deselect() {
    HAL_GPIO_WritePin(MCP_CS_PORT, MCP_CS_PIN, GPIO_PIN_SET);
}

// --- SPI Byte Transfer ---
static uint8_t mcp_transfer(SPI_HandleTypeDef* hspi, uint8_t data) {
    uint8_t recv = 0;
    HAL_SPI_TransmitReceive(hspi, &data, &recv, 1, HAL_MAX_DELAY);
    return recv;
}

// --- Reset the MCP2515 ---
void mcp2515_reset(SPI_HandleTypeDef* hspi) {
    mcp_select();
    mcp_transfer(hspi, MCP_RESET);
    mcp_deselect();
    HAL_Delay(10);
}

// --- Write to a register ---
void mcp2515_write_register(SPI_HandleTypeDef* hspi, uint8_t reg, uint8_t value) {
    mcp_select();
    mcp_transfer(hspi, MCP_WRITE);
    mcp_transfer(hspi, reg);
    mcp_transfer(hspi, value);
    mcp_deselect();
}

// --- Read from a register ---
uint8_t mcp2515_read_register(SPI_HandleTypeDef* hspi, uint8_t reg) {
    mcp_select();
    mcp_transfer(hspi, MCP_READ);
    mcp_transfer(hspi, reg);
    uint8_t result = mcp_transfer(hspi, 0xFF);
    mcp_deselect();
    return result;
}

// --- Bit modify (mask + write bits) ---
void mcp2515_bit_modify(SPI_HandleTypeDef* hspi, uint8_t reg, uint8_t mask, uint8_t data) {
    mcp_select();
    mcp_transfer(hspi, MCP_BITMOD);
    mcp_transfer(hspi, reg);
    mcp_transfer(hspi, mask);
    mcp_transfer(hspi, data);
    mcp_deselect();
}

// --- Read status (TX/RX flags etc.) ---
uint8_t mcp2515_read_status(SPI_HandleTypeDef* hspi) {
    mcp_select();
    mcp_transfer(hspi, MCP_READ_STATUS);
    uint8_t status = mcp_transfer(hspi, 0xFF);
    mcp_deselect();
    return status;
}

// --- Set operating mode ---
void mcp2515_set_mode(SPI_HandleTypeDef* hspi, uint8_t mode) {
    mcp2515_bit_modify(hspi, MCP_CANCTRL, 0xE0, mode);
    HAL_Delay(10);
    while ((mcp2515_read_register(hspi, MCP_CANSTAT) & 0xE0) != mode);
}

// --- MCP2515 Initialization ---
void mcp2515_init(SPI_HandleTypeDef* hspi) {
    mcp2515_reset(hspi);

    // Enter config mode
    mcp2515_set_mode(hspi, MODE_CONFIG);

    // Set bit timing (for 500 kbps @ 16 MHz crystal)
    mcp2515_write_register(hspi, MCP_CNF1, 0x00);
    mcp2515_write_register(hspi, MCP_CNF2, 0x90);
    mcp2515_write_register(hspi, MCP_CNF3, 0x02);

    // Enter loopback mode for testing
    mcp2515_set_mode(hspi, MODE_LOOPBACK);
}

// --- Send one CAN frame (ID 0x123, data 01 02 03 04) ---
void mcp2515_send_test_frame(SPI_HandleTypeDef* hspi) {
    // Write 11-bit ID = 0x123 (SIDH/SIDL)
    mcp2515_write_register(hspi, MCP_TXB0SIDH, 0x24);  // 0x123 >> 3
    mcp2515_write_register(hspi, MCP_TXB0SIDL, 0x60);  // (0x123 << 5) & 0xE0

    // Write DLC = 4
    mcp2515_write_register(hspi, MCP_TXB0DLC, 4);

    // Write 4 bytes of data
    mcp2515_write_register(hspi, MCP_TXB0D0, 0x01);
    mcp2515_write_register(hspi, MCP_TXB0D1, 0x02);
    mcp2515_write_register(hspi, MCP_TXB0D2, 0x03);
    mcp2515_write_register(hspi, MCP_TXB0D3, 0x04);

    // Request transmission
    mcp_select();
    mcp_transfer(hspi, MCP_RTS_TX0);
    mcp_deselect();
}
