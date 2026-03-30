#include "http_client.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "cJSON.h"
#include "nvs_storage.h"
#include "wifi_manager.h"
#include <string.h>
#include <time.h>

static const char *TAG = "http_client";

static char claim_response_buf[256];
static int claim_response_len = 0;

static char poll_response_buf[512];
static int poll_response_len = 0;

static esp_err_t claim_http_event_handler(esp_http_client_event_t *evt) {
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        int copy_len = evt->data_len;
        if (claim_response_len + copy_len >= (int)sizeof(claim_response_buf)) {
            copy_len = sizeof(claim_response_buf) - claim_response_len - 1;
        }
        memcpy(claim_response_buf + claim_response_len, evt->data, copy_len);
        claim_response_len += copy_len;
        claim_response_buf[claim_response_len] = '\0';
    }
    return ESP_OK;
}

static esp_err_t poll_http_event_handler(esp_http_client_event_t *evt) {
    if (evt->event_id == HTTP_EVENT_ON_DATA) {
        int copy_len = evt->data_len;
        if (poll_response_len + copy_len >= (int)sizeof(poll_response_buf)) {
            copy_len = sizeof(poll_response_buf) - poll_response_len - 1;
        }
        memcpy(poll_response_buf + poll_response_len, evt->data, copy_len);
        poll_response_len += copy_len;
        poll_response_buf[poll_response_len] = '\0';
    }
    return ESP_OK;
}

esp_err_t http_client_init(void) {
    if (!wifi_manager_is_connected()) {
        ESP_LOGE(TAG, "WiFi not connected, cannot send scan");
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t http_client_claim_device(char *claim_token, size_t token_len) {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char mac_str[32];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "mac_address", mac_str);
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    claim_response_len = 0;
    memset(claim_response_buf, 0, sizeof(claim_response_buf));

    esp_http_client_config_t config = {
        .url = API_BASE_URL API_CLAIM_ENDPOINT,
        .method = HTTP_METHOD_POST,
        .timeout_ms = API_TIMEOUT_MS,
        .skip_cert_common_name_check = true,
        .use_global_ca_store = false,
        .event_handler = claim_http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t ret = esp_http_client_perform(client);
    free(payload);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Claim request failed: %s", esp_err_to_name(ret));
        esp_http_client_cleanup(client);
        return ret;
    }

    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (status != 200) {
        ESP_LOGE(TAG, "Claim failed, HTTP status: %d", status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Claim response: %s", claim_response_buf);

    cJSON *response = cJSON_Parse(claim_response_buf);
    if (response) {
        cJSON *token = cJSON_GetObjectItem(response, "claim_token");
        if (token && cJSON_IsString(token)) {
            strncpy(claim_token, token->valuestring, token_len - 1);
            claim_token[token_len - 1] = '\0';
            ESP_LOGI(TAG, "Device claimed, token %s", claim_token);
        } else {
            ESP_LOGE(TAG, "No claim_token in response");
            cJSON_Delete(response);
            return ESP_FAIL;
        }
        cJSON_Delete(response);
    }

    return ESP_OK;
}

esp_err_t http_client_poll_device(const char *claim_token, char *device_id, size_t id_len, char *api_key, size_t key_len, bool *linked) {
    char url[256];
    snprintf(url, sizeof(url), "%s%s/%s", API_BASE_URL, API_POLL_ENDPOINT, claim_token);

    poll_response_len = 0;
    memset(poll_response_buf, 0, sizeof(poll_response_buf));

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = API_TIMEOUT_MS,
        .skip_cert_common_name_check = true,
        .use_global_ca_store = false,
        .event_handler = poll_http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t ret = esp_http_client_perform(client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Poll request failed: %s", esp_err_to_name(ret));
        esp_http_client_cleanup(client);
        return ret;
    }

    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (status != 200) {
        ESP_LOGE(TAG, "Poll failed, HTTP status: %d", status);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Poll response: %s", poll_response_buf);

    cJSON *response = cJSON_Parse(poll_response_buf);
    if (response) {
        cJSON *linked_field = cJSON_GetObjectItem(response, "linked");
        *linked = cJSON_IsTrue(linked_field);

        if (*linked) {
            cJSON *id = cJSON_GetObjectItem(response, "device_id");
            cJSON *key = cJSON_GetObjectItem(response, "api_key");
            if (id && key) {
                snprintf(device_id, id_len, "%d", id->valueint);
                strncpy(api_key, key->valuestring, key_len - 1);
                api_key[key_len - 1] = '\0';
                ESP_LOGI(TAG, "Device linked: id=%s", device_id);
            }
        } else {
            ESP_LOGI(TAG, "Device not yet linked, polling...");
        }
        cJSON_Delete(response);
    }

    return ESP_OK;
}

esp_err_t http_client_send_scan(const char *barcode, const char *action, char *product_name, size_t buf_len) {
    char api_key[128];
    esp_err_t nvs_ret = nvs_read_api_key(api_key, sizeof(api_key));
    if (nvs_ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read API key from NVS");
        return nvs_ret;
    }

    char timestamp[32];
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%S", t);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "barcode", barcode);
    cJSON_AddStringToObject(root, "action", action);
    cJSON_AddStringToObject(root, "timestamp", timestamp);
    cJSON_AddStringToObject(root, "source", "device");
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    esp_http_client_config_t config = {
        .url = API_BASE_URL API_SCAN_ENDPOINT,
        .method = HTTP_METHOD_POST,
        .timeout_ms = API_TIMEOUT_MS,
        .skip_cert_common_name_check = true,
        .use_global_ca_store = false,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "X-Api-Key", api_key);
    esp_http_client_set_post_field(client, payload, strlen(payload));

    esp_err_t ret = esp_http_client_perform(client);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Scan request failed: %s", esp_err_to_name(ret));
        free(payload);
        esp_http_client_cleanup(client);
        return ret;
    }

    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "Scan HTTP response: %d", status_code);

    char response_buf[256];
    int response_len = esp_http_client_read_response(client, response_buf, sizeof(response_buf) - 1);
    if (response_len < 0) response_len = 0;
    response_buf[response_len] = '\0';

    cJSON *response = cJSON_Parse(response_buf);
    if (response) {
        cJSON *name = cJSON_GetObjectItem(response, "product_name");
        if (name && cJSON_IsString(name)) {
            strncpy(product_name, name->valuestring, buf_len - 1);
            product_name[buf_len - 1] = '\0';
        }
        cJSON_Delete(response);
    }

    free(payload);
    esp_http_client_cleanup(client);
    return (status_code == 200) ? ESP_OK : ESP_FAIL;
}