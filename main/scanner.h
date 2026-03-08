#pragma once

#include "esp_err.h"
#include "config.h"

esp_err_t scanner_init(void);

esp_err_t scanner_read_barcode(char *buf, size_t buf_len);