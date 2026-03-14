#include "led_strip.h"
#include "led.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "led";
static led_strip_handle_t led_strip;
static TaskHandle_t breathing_task_handle = NULL;
static uint8_t breath_r, breath_g, breath_b;
static volatile bool stop_breath = false;

static void stop_breathing(void) {
    if (breathing_task_handle != NULL) {
        stop_breath = true;
        vTaskDelay(pdMS_TO_TICKS(100));
        breathing_task_handle = NULL;
        stop_breath = false;
    }
}

static void breathing_task(void *arg) {
    uint8_t brightness = 0;
    int8_t direction = 1;

    while (!stop_breath) {
        float scale = brightness / 255.0f;
        led_strip_set_pixel(led_strip, 0,
            (uint8_t)(breath_r * scale),
            (uint8_t)(breath_g * scale),
            (uint8_t)(breath_b * scale));
        led_strip_refresh(led_strip);

        brightness += direction * 3;
        if (brightness >= 255) {
            brightness = 255;
            direction = -1;
        } else if (brightness <= 0) {
            brightness = 0;
            direction = 1;
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelete(NULL);
}

esp_err_t led_init(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = ONBOARD_RGB_PIN,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
    };
    esp_err_t ret = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LED init failed");
        return ret;
    }
    ESP_LOGI(TAG, "LED initialized on GPIO%d", ONBOARD_RGB_PIN);
    return ESP_OK;
}

esp_err_t led_set_color(uint8_t r, uint8_t g, uint8_t b) {
    stop_breathing();
    esp_err_t ret = led_strip_set_pixel(led_strip, 0, r, g, b);
    if (ret != ESP_OK) return ret;
    return led_strip_refresh(led_strip);
}

esp_err_t led_set_idle(void) {
    stop_breathing();
    breath_r = 50; breath_g = 50; breath_b = 80;
    xTaskCreate(breathing_task, "led_breath", 2048, NULL, 3, &breathing_task_handle);
    return ESP_OK;
}

esp_err_t led_set_error(void) {
    stop_breathing();
    breath_r = 255; breath_g = 0; breath_b = 0;
    xTaskCreate(breathing_task,"led_breath", 2048, NULL, 3, &breathing_task_handle);
    return ESP_OK;
}

esp_err_t led_off(void) {
    stop_breathing();
    led_strip_set_pixel(led_strip, 0, 0, 0, 0);
    led_strip_refresh(led_strip);
    return led_strip_clear(led_strip);
}

esp_err_t led_fade_out(void) {
    stop_breathing();

    for (int brightness = 255; brightness >= 0; brightness -= 5) {
        float scale = brightness / 255.0f;
        led_strip_set_pixel(led_strip, 0,
            (uint8_t)(breath_r * scale),
            (uint8_t)(breath_g * scale),
            (uint8_t)(breath_b * scale));
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
    return led_strip_clear(led_strip);
}