#pragma once

#include <stdbool.h>
#include "config.h"
#include "esp_err.h"

esp_err_t nvs_init(void);
esp_err_t nvs_write_wifi_credentials(const char *ssid, const char *pass);
esp_err_t nvs_read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len);
esp_err_t nvs_write_device_id(const char *id);
esp_err_t nvs_read_device_id(char *id, size_t id_len);
esp_err_t nvs_write_api_key(const char *api_key);
esp_err_t nvs_read_api_key(char *api_key, size_t key_len);
esp_err_t nvs_write_provisioned(bool provisioned);
esp_err_t nvs_read_provisioned(bool *provisioned);
esp_err_t nvs_reset_credentials(void);