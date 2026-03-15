#include "driver/ledc.h"
#include "buzzer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "buzzer";

esp_err_t buzzer_init(void) {
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    esp_err_t ret = ledc_timer_config(&timer);
    if (ret != ESP_OK) return ret;

    ledc_channel_config_t channel = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ret = ledc_channel_config(&channel);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Buzzer initialized on GPIO%d", BUZZER_PIN);
    return ESP_OK;
}

esp_err_t buzzer_beep(uint32_t frequency, uint32_t duration_ms) {
    ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequency);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    return ESP_OK;
}

esp_err_t buzzer_success(void) {
    buzzer_beep(2000, 100);
    vTaskDelay(pdMS_TO_TICKS(50));
    buzzer_beep(2500, 100);
    return ESP_OK;
}

esp_err_t buzzer_fail(void) {
    buzzer_beep(800, 150);
    vTaskDelay(pdMS_TO_TICKS(50));
    buzzer_beep(500, 300);
    return ESP_OK;
}

esp_err_t buzzer_boot(void) {
    buzzer_beep(1000, 100);
    vTaskDelay(pdMS_TO_TICKS(50));
    buzzer_beep(1500, 100);
    vTaskDelay(pdMS_TO_TICKS(50));
    buzzer_beep(2000, 100);
    return ESP_OK;
}