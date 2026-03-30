#pragma once

#include "esp_err.h"
#include "config.h"
#include <stdbool.h>

esp_err_t http_client_init(void);
esp_err_t http_client_claim_device(char *claim_token, size_t token_len);
esp_err_t http_client_poll_device(const char *claim_token, char *device_id, size_t id_len, char *api_key, size_t key_len, bool *linked);
esp_err_t http_client_send_scan(const char *barcode, const char *action, char *product_name, size_t buf_len);