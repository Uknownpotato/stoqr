#pragma once

#include "esp_err.h"
#include "config.h"

struct QueueDefinition;
typedef struct QueueDefinition * QueueHandle_t;

void scanner_task_start(QueueHandle_t queue);
esp_err_t scanner_init(void);
esp_err_t scanner_read_barcode(char *buf, size_t buf_len);