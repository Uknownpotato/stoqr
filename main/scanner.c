#include "scanner.h"
#include "uart.h"
#include "esp_log.h"

static const char *TAG = "scanner";

esp_err_t scanner_init(void) {
    esp_err_t ret;
    
    ret = uart_scanner_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Scanner init failed");
        return ret;
    }

    ESP_LOGI(TAG, "Scanner initialized");
    return ESP_OK;
}

esp_err_t scanner_read_barcode(char *buf, size_t buf_len) {
    size_t index = 0;
    uint8_t byte;

    while (index < buf_len - 1) {
        int result = uart_scanner_read(&byte, 1, pdMS_TO_TICKS(5000));

        if (result <= 0) return ESP_ERR_TIMEOUT;
        if (byte == '\n' || byte == '\r') break;

        buf[index] = byte;
        index++;
    }

    buf[index] = '\0';
    ESP_LOGI(TAG, "Barcode scanned: %s", buf);

    return ESP_OK;
}