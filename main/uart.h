#pragma once

#include <stdint.h>
#include "driver/uart.h"
#include "config.h"

// Initialize UART peripheral for GM65 scanner
esp_err_t uart_scanner_init(void);

// Read a line from UART (blocking until newline or timeout)
int uart_scanner_read(uint8_t *buf, size_t max_len, TickType_t timeout_ticks);

// Write raw bytes to UART
int uart_scanner_write(const uint8_t *data, size_t len);

// Flush UART receive buffer
void uart_scanner_flush(void);
