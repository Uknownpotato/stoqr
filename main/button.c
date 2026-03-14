#include "button.h"
#include "config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "button";
static button_event_t current_event = BUTTON_EVENT_NONE;
static button_mode_t current_mode = BUTTON_MODE_ADD;

static void button_task(void *arg) {
    while (1) {
        if (gpio_get_level(BUTTON_PIN) == 0) {
            TickType_t press_time = xTaskGetTickCount();

            while (gpio_get_level(BUTTON_PIN) == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }

            TickType_t held_ms = (xTaskGetTickCount() - press_time) * portTICK_PERIOD_MS;
        
            if (held_ms >= 2000) {
                current_event = BUTTON_EVENT_LONG_PRESS;
            } else {
                TickType_t wait_start = xTaskGetTickCount();
                bool second_click = false;

                while ((xTaskGetTickCount() - wait_start) * portTICK_PERIOD_MS < BUTTON_DOUBLE_CLICK_MS) {
                    if (gpio_get_level(BUTTON_PIN) == 0) {
                        second_click = true;

                        while (gpio_get_level(BUTTON_PIN) == 0) {
                            vTaskDelay(pdMS_TO_TICKS(10));
                        }
                        break;
                    }
                    vTaskDelay(pdMS_TO_TICKS(10));
                }

                if (second_click) {
                    current_event = BUTTON_EVENT_DOUBLE_CLICK;
                    current_mode = BUTTON_MODE_REMOVE;
                } else {
                    current_event = BUTTON_EVENT_SINGLE_CLICK;
                    current_mode = BUTTON_MODE_ADD;
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

esp_err_t button_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed");
        return ret;
    }
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    ESP_LOGI(TAG, "Button initialized on GPIO%d", BUTTON_PIN);
    return ESP_OK;
}

button_event_t button_get_event(void) {
    button_event_t event = current_event;
    current_event = BUTTON_EVENT_NONE;
    return event;
}

button_mode_t button_get_mode(void) {
    return current_mode;
}