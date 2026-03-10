#pragma once

#include "esp_err.h"
#include "config.h"

esp_err_t http_client_init(void);

esp_err_t http_client_send_scan(const char *barcode, const char *action, char *product_name, size_t buf_len);