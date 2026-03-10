#include "http_client.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "nvs_storage.h"
#include "wifi_manager.h"
#include <string.h>

static const char *TAG = "http_client";

esp_err_t http_client_init(void) {
    if (!wifi_manager_is_connected()) {
        ESP_LOGE(TAG, "WiFi not connected - cannot send scan");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t http_client_send_scan(const char *barcode, const char *action, char *product_name, size_t buf_len) {
    char device_id[32];
    nvs_read_device_id(device_id, sizeof(device_id));

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", device_id);
    cJSON_AddStringToObject(root, "barcode", barcode);
    cJSON_AddStringToObject(root, "action", action);
    cJSON_AddStringToObject(root, "source", "device");
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    esp_http_client_config_t config = {
        .url = API_BASE_URL API_SCAN_ENDPOINT,
        .method = HTTP_METHOD_POST,
        .timeout_ms = API_TIMEOUT_MS,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));
    esp_err_t ret = esp_http_client_perform(client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(ret));
        free(payload);
        esp_http_client_cleanup(client);
        return ret;
    }

    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP response: %d", status_code);

    char response_buf[256];
    int response_len = esp_http_client_read_response(client, response_buf, sizeof(response_buf) - 1);
    response_buf[response_len] = '\0';

    cJSON *response = cJSON_Parse(response_buf);
    if (response) {
        cJSON *name = cJSON_GetObjectItem(response, "product_name");
        if (name && cJSON_IsString(name)) {
            strncpy(product_name, name->valuestring, buf_len - 1);
        }
        cJSON_Delete(response);
    }

    free(payload);
    esp_http_client_cleanup(client);
    return ESP_OK;
}