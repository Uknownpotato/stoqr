#include <stdio.h>
#include "uart.h"
#include "buzzer.h"
#include "scanner.h"
#include "button.h"
#include "led.h"
#include "provisioning.h"
#include "nvs_storage.h"
#include "wifi_manager.h"
#include "http_client.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "main";

void app_main(void)
{
    ESP_LOGI(TAG, "Stoqr firmware starting...");

    if (led_init() != ESP_OK) return;
    if (buzzer_init() != ESP_OK) return;
    if (scanner_init() != ESP_OK) return;
    if (button_init() != ESP_OK) return;

    nvs_init();
    
    bool provisioned = false;
    nvs_read_provisioned(&provisioned);

    if (!provisioned) {
        ESP_LOGI(TAG, "Not provisioned. Starting setup mode");
        provisioning_start();
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "Already provisioned. Starting normal mode");
    buzzer_boot();
    led_set_idle();

    char ssid[64], pass[64];
    nvs_read_wifi_credentials(ssid, sizeof(ssid), pass, sizeof(pass));

    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);
    wifi_manager_init();
    esp_err_t wifi_ret = wifi_manager_connect(ssid, pass);
    if (wifi_ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connection failed. Check credentials");
        led_set_error();
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
    }

    http_client_init();

    QueueHandle_t barcode_queue = xQueueCreate(10, 64);
    scanner_task_start(barcode_queue);

    char barcode[64];

    while (1) {
        button_event_t event = button_get_event();

        if (event  == BUTTON_EVENT_SINGLE_CLICK) {
            ESP_LOGI(TAG, "Single click, ADD mode");
            buzzer_success();
        } else if (event == BUTTON_EVENT_DOUBLE_CLICK) {
            ESP_LOGI(TAG, "Double click, REMOVE mode");
        } else if (event == BUTTON_EVENT_LONG_PRESS) {
            ESP_LOGI(TAG, "Long press, going to sleep");
            led_fade_out();
        }

        if (xQueueReceive(barcode_queue, barcode, 0) == pdTRUE) {
            ESP_LOGI(TAG, "Got barcode: %s", barcode);
            char product_name[128];
            button_mode_t mode = button_get_mode();
            const char *action = (mode == BUTTON_MODE_ADD) ? "add" : "remove";

            esp_err_t ret = http_client_send_scan(barcode, action, product_name, sizeof(product_name));
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Product: %s", product_name);
                led_set_color(0, 255, 0);
                buzzer_success();
            } else {
                led_set_color(255, 0 ,0);
                buzzer_fail();
            }
            
            vTaskDelay(pdMS_TO_TICKS(1000));
            led_set_idle();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
