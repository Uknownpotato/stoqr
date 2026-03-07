#pragma once

#include "secrets.h"

// UART / GM65 Scanner
#define SCANNER_UART_NUM            UART_NUM_1
#define SCANNER_TX_PIN              17
#define SCANNER_RX_PIN              18
#define SCANNER_BAUD_RATE           9600
#define SCANNER_BUF_SIZE            256

// I2C / OLED Display
#define OLED_I2C_NUM                I2C_NUM_0
#define OLED_SDA_PIN                8
#define OLED_SCL_PIN                9
#define OLED_I2C_FREQ_HZ            400000
#define OLED_I2C_ADDR               0x3c

// Button
#define BUTTON_PIN                  4
#define BUTTON_SINGLE_CLICK_MS      300
#define BUTTON_DOUBLE_CLICK_MS      400

// LED
#define LED_PIN                     5
#define ONBOARD_RGB_PIN             38

// Power Management
#define DEEP_SLEEP_TIMEOUT_MS       30000

// NVS
#define NVS_NAMESPACE               "stoqr"

// HTTP API
#define API_BASE_URL                "https://api.stoqr.app"
#define API_SCAN_ENDPOINT           "/scan"
#define API_TIMEOUT_MS              10000

// WiFi Provisioning
#define PROV_AP_SSID                "Stoqr-Setup"
#define PROV_AP_PASS                "stoqr1234"
#define PROV_TIMEOUT_MS             120000