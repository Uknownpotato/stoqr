#pragma once

#include "esp_err.h"

typedef enum {
    BUTTON_EVENT_NONE,
    BUTTON_EVENT_SINGLE_CLICK,
    BUTTON_EVENT_DOUBLE_CLICK,
    BUTTON_EVENT_LONG_PRESS
} button_event_t;

typedef enum {
    BUTTON_MODE_ADD,
    BUTTON_MODE_REMOVE
} button_mode_t;

esp_err_t button_init(void);

button_event_t button_get_event(void);

button_mode_t button_get_mode(void);