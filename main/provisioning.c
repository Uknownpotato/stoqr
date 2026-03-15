#include "provisioning.h"
#include "config.h"
#include "nvs_storage.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "provisioning";
static httpd_handle_t server = NULL;

static const char *HTML_PAGE =
    "<html><body>"
    "<h2>Stoqr Setup</h2>"
    "<form method='POST' action='/save'>"
    "WiFi Name: <input name='ssid'><br><br>"
    "Password: <input name='pass' type='password'><br><br>"
    "<input type='submit' value='Connect'>"
    "</form></body></html>";

static void url_decode(char *str) {
    char *src = str, *dst = str;
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            char hex[3] = {src[1], src[2], 0};
            *dst++ = (char)strtol(hex, NULL, 16);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, HTML_PAGE, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t save_post_handler(httpd_req_t *req) {
    char body[256];
    int ret = httpd_req_recv(req, body, sizeof(body) - 1);
    if (ret <= 0) return ESP_FAIL;
    body[ret] = '\0';

    char ssid[64] = {0};
    char pass[64] = {0};

    char *ssid_start = strstr(body, "ssid=");
    char *pass_start = strstr(body, "pass=");

    if (ssid_start && pass_start) {
        ssid_start += 5;
        pass_start += 5;

        char *end = strstr(ssid_start, "&");
        if (end) strncpy(ssid, ssid_start, end - ssid_start);
        strncpy(pass, pass_start, sizeof(pass) - 1);

        url_decode(ssid);
        url_decode(pass);

        nvs_write_wifi_credentials(ssid, pass);
        nvs_write_provisioned(true);
        httpd_resp_sendstr(req, "<html><body><h2>Saved! Rebooting...</h2></body></html>");
        esp_restart();
    }
    return ESP_OK;
}

esp_err_t provisioning_start(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t ap_config = {
        .ap = {
            .ssid = PROV_AP_SSID,
            .password = PROV_AP_PASS,
            .max_connection = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
    esp_wifi_start();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_start(&server, &config);

    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
    };
    httpd_register_uri_handler(server, &root);

    httpd_uri_t save = {
        .uri = "/save",
        .method = HTTP_POST,
        .handler = save_post_handler,
    };
    httpd_register_uri_handler(server, &save);

    ESP_LOGI(TAG, "Provisioning started, connect to '%s'", PROV_AP_SSID);
    return ESP_OK;
}

esp_err_t provisioning_stop(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
    esp_wifi_stop();
    ESP_LOGI(TAG, "Provisioning stopped");
    return ESP_OK;
}