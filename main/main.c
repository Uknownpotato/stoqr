#include <stdio.h>
#include "uart.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Stoqr firmware starting...");

    if (uart_scanner_init() == ESP_OK) {
        ESP_LOGI(TAG, "UART ready");
    } else {
        ESP_LOGE(TAG, "UART init failed");
    }
}
