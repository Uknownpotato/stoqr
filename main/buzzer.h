#pragma once

#include "esp_err.h"
#include "config.h"

esp_err_t buzzer_init(void);
esp_err_t buzzer_beep(uint32_t frequency, uint32_t duration_ms);
esp_err_t buzzer_success(void);
esp_err_t buzzer_fail(void);
esp_err_t buzzer_boot(void);