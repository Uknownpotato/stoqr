#include "nvs_storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "NVS";

esp_err_t nvs_init(void) {
    esp_err_t ret;

    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NEW_VERSION_FOUND || ret == ESP_ERR_NOT_FOUND) {
        ret = nvs_flash_erase();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "NVS erase failed");
            return ret;
        }

        ret = nvs_flash_init();
    }

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed");
        return ret;
    }

    ESP_LOGI(TAG, "NVS initialized");
    return ESP_OK;
}

esp_err_t nvs_write_wifi_credentials(const char *ssid, const char *pass) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READWRITE wifi credentials failed");
        return ret;
    }

    ret = nvs_set_str(handle, "wifi_ssid", ssid);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set SSID failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_set_str(handle, "wifi_pass", pass);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set password failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS wifi credentials commit failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_read_wifi_credentials(char *ssid, size_t ssid_len, char *pass, size_t pass_len) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open wifi credentials READONLY failed");
        return ret;
    }

    ret = nvs_get_str(handle, "wifi_ssid", ssid, &ssid_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS get SSID failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_get_str(handle, "wifi_pass", pass, &pass_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS get password failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_write_device_id(const char* id) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READWRITE deviceID failed");
        return ret;
    }

    ret = nvs_set_str(handle, "device_id", id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set deviceID failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS deviceID commit failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_read_device_id(char *id, size_t id_len) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open deviceID READONLY failed");
        return ret;
    }

    ret = nvs_get_str(handle, "device_id", id, &id_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS get deviceID failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_write_api_key(const char *api_key) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READWRITE API failed");
        return ret;
    }

    ret = nvs_set_str(handle, "api_key", api_key);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set API failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS API commit failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK; 
}

esp_err_t nvs_read_api_key(char *api, size_t api_len) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READONLY API failed");
        return ret;
    }

    ret = nvs_get_str(handle, "api_key", api, &api_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS get API failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_write_provisioned(bool provisioned) {
    nvs_handle_t handle;
    esp_err_t ret;
    uint8_t value = provisioned ? 1 : 0;

    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READWRITE provisioned failed");
        return ret;
    }

    ret = nvs_set_u8(handle, "provisioned", value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set provisioned failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS provisioned commit failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_read_provisioned(bool *provisioned) {
    nvs_handle_t handle;
    esp_err_t ret;
    uint8_t value;

    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READONLY provisioned failed");
        return ret;
    }

    ret = nvs_get_u8(handle, "provisioned", &value);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS get provisioned failed");
        nvs_close(handle);
        return ret;
    }

    *provisioned = (bool)value;

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_reset_credentials(void) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS reset open failed");
        return ret;
    }

    ret = nvs_erase_all(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS erase all failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS reset commit failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t nvs_write_claim_token(const char *token) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READWRITE claim_token failed");
        return ret;
    }

    ret = nvs_set_str(handle, "claim_token", token);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS set claim_token failed");
        nvs_close(handle);
        return ret;
    }

    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS claim_token commit failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK; 
}

esp_err_t nvs_read_claim_token(char *token, size_t token_len) {
    nvs_handle_t handle;
    esp_err_t ret;

    ret = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS open READONLY claim_token failed");
        return ret;
    }

    ret = nvs_get_str(handle, "claim_token", token, &token_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "NVS get claim_token failed");
        nvs_close(handle);
        return ret;
    }

    nvs_close(handle);
    return ESP_OK;
}