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
        led_set_color(0, 0, 255);
        provisioning_start();
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGI(TAG, "Already provisioned. Starting normal mode");
    buzzer_boot();
    led_set_idle();

    char ssid[64], pass[64];
    nvs_read_wifi_credentials(ssid, sizeof(ssid), pass, sizeof(pass));

    ESP_LOGI(TAG, "Connecting to WiFi: %s", ssid);
    led_set_color(255, 165, 0);
    wifi_manager_init();
    esp_err_t wifi_ret = wifi_manager_connect(ssid, pass);
    if (wifi_ret != ESP_OK) {
        ESP_LOGE(TAG, "WiFi connection failed. Check credentials");
        led_set_error();
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
    }

    http_client_init();

    char api_key[128] = {0};
    esp_err_t key_ret = nvs_read_api_key(api_key, sizeof(api_key));

    if (key_ret != ESP_OK || strlen(api_key) == 0) {
        ESP_LOGI(TAG, "Device not linked. Starting claim flow...");
        led_set_color(0, 0, 255);

        char claim_token[64] = {0};
        esp_err_t token_ret = nvs_read_claim_token(claim_token, sizeof(claim_token));

        if (token_ret != ESP_OK || strlen(claim_token) == 0) {
            esp_err_t claim_ret = http_client_claim_device(claim_token, sizeof(claim_token));
            if (claim_ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to claim device. Rebooting...");
                led_set_error();
                vTaskDelay(pdMS_TO_TICKS(5000));
                esp_restart();
            }
            nvs_write_claim_token(claim_token);
            ESP_LOGI(TAG, "Device claimed, token: %s", claim_token);
        } else {
            ESP_LOGI(TAG, "Resuming poll with existing token: %s", claim_token);
        }

        bool linked = false;
        char device_id[16] = {0};
        char new_api_key[128] = {0};

        while (!linked) {
            ESP_LOGI(TAG, "Polling for device link...");
            esp_err_t poll_ret = http_client_poll_device(claim_token, device_id, sizeof(device_id), new_api_key, sizeof(new_api_key), &linked);
            if (poll_ret != ESP_OK) {
                ESP_LOGW(TAG, "Poll failed, retrying...");
            }
            if (!linked) {
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }

        nvs_write_device_id(device_id);
        nvs_write_api_key(new_api_key);
        ESP_LOGI(TAG, "Device linked! id=%s", device_id);
        buzzer_success();
        led_set_idle();
    }

    ESP_LOGI(TAG, "Device ready. Starting scan loop");

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
