#include "uart.h"
#include "esp_log.h"

static const char *TAG = "uart";

esp_err_t uart_scanner_init(void) {
    uart_config_t uart_config = {
        .baud_rate  = SCANNER_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };

    esp_err_t ret;

    ret = uart_param_config(SCANNER_UART_NUM, &uart_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters");
        return ret;
    }

    ret = uart_set_pin(
            SCANNER_UART_NUM,
            SCANNER_TX_PIN,
            SCANNER_RX_PIN,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins");
        return ret;
    }

    ret = uart_driver_install(
            SCANNER_UART_NUM,
            SCANNER_BUF_SIZE * 2,
            0,
            0,
            NULL,
            0
        );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        return ret;
    }

    ESP_LOGI(TAG, "UART initialized on TX%d RX%d at %d baud",
                SCANNER_TX_PIN, SCANNER_RX_PIN, SCANNER_BAUD_RATE);
    
    return ESP_OK;
}

int uart_scanner_read(uint8_t *buf, size_t max_len, TickType_t timeout_ticks) {
    return uart_read_bytes(SCANNER_UART_NUM, buf, max_len, timeout_ticks);
}

int uart_scanner_write(const uint8_t *data, size_t len) {
    return uart_write_bytes(SCANNER_UART_NUM, (const char*)data, len);
}

void uart_scanner_flush(void) {
    uart_flush(SCANNER_UART_NUM);
}