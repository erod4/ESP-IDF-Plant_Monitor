#ifndef WIFI_APP_H
#define WIFI_APP_H
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_netif.h"

// Callback typedef
typedef void (*wifi_connected_event_callback_t)(void);

#define WIFI_STA_POWER_SAVE WIFI_PS_NONE // Power save not used
#define MAX_SSID_LENGTH 32               // IEEE standard maximum
#define MAX_PASSWORD_LENGTH 64           // IEEE standard maximum
#define MAX_CONNECTION_RETRIES 5         // Retry number on disconnect
/**
 * Initializes WiFi application
 */
void wifi_init(void);

/**
 * Starts WiFi application
 */
esp_err_t wifi_connect_sta();

/**
 * Sets callback function once wifi has IP
 */
void wifi_app_set_callback(wifi_connected_event_callback_t cb);

/**
 * Calls the callback function
 */
void wifi_app_call_callback(void);
#endif
