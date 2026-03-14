#include <stdio.h>
#include "uart.h"
#include "scanner.h"
#include "button.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Stoqr firmware starting...");

    if (scanner_init() != ESP_OK) {
        return;
    }

    if (button_init() != ESP_OK) {
        return;
    }

    char barcode[64];

    while (1) {
        button_event_t event = button_get_event();
        esp_err_t ret = scanner_read_barcode(barcode, sizeof(barcode));

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Got barcode: %s", barcode);
        } else {
            ESP_LOGW(TAG, "No scan yet...");
        }

        if (event  == BUTTON_EVENT_SINGLE_CLICK) {
            ESP_LOGI(TAG, "Single click, ADD mode");
        } else if (event == BUTTON_EVENT_DOUBLE_CLICK) {
            ESP_LOGI(TAG, "Double click, REMOVE mode");
        } else if (event == BUTTON_EVENT_LONG_PRESS) {
            ESP_LOGI(TAG, "Long press, going to sleep");
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
