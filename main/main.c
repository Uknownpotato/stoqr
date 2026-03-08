#include <stdio.h>
#include "uart.h"
#include "scanner.h"
#include "esp_log.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Stoqr firmware starting...");

    if (scanner_init() != ESP_OK) {
        return;
    }

    char barcode[64];

    while (1) {
        esp_err_t ret = scanner_read_barcode(barcode, sizeof(barcode));
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Got barcode: %s", barcode);
        } else {
            ESP_LOGW(TAG, "No scan yet...");
        }
    }
}
