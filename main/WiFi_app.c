#include "WiFi_app.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_netif.h"
#include <string.h>
#include "app_NVS.h"
#include "rgb_led.h"
#include "global_event_group.h"
#include "aws_iot.h"
#include "toggle_sleep_button.h"

static esp_netif_t *esp_netif;
static const char TAG[] = "WIFI_APP";
static EventGroupHandle_t wifi_events;
const int CONNECTED = BIT0;
const int DISCONNECT = BIT1;

TimerHandle_t wifi_connection_timer;
int wifi_retry_count = 0;

static wifi_connected_event_callback_t wifi_connected_event_cb;

static char *get_wifi_disconnection_str(wifi_err_reason_t wifi_err_reason)
{
    switch (wifi_err_reason)
    {
    case WIFI_REASON_UNSPECIFIED:
        return "WIFI_REASON_UNSPECIFIED";
    case WIFI_REASON_AUTH_EXPIRE:
        return "WIFI_REASON_AUTH_EXPIRE";
    case WIFI_REASON_AUTH_LEAVE:
        return "WIFI_REASON_AUTH_LEAVE";
    case WIFI_REASON_ASSOC_EXPIRE:
        return "WIFI_REASON_ASSOC_EXPIRE";
    case WIFI_REASON_ASSOC_TOOMANY:
        return "WIFI_REASON_ASSOC_TOOMANY";
    case WIFI_REASON_NOT_AUTHED:
        return "WIFI_REASON_NOT_AUTHED";
    case WIFI_REASON_NOT_ASSOCED:
        return "WIFI_REASON_NOT_ASSOCED";
    case WIFI_REASON_ASSOC_LEAVE:
        return "WIFI_REASON_ASSOC_LEAVE";
    case WIFI_REASON_ASSOC_NOT_AUTHED:
        return "WIFI_REASON_ASSOC_NOT_AUTHED";
    case WIFI_REASON_DISASSOC_PWRCAP_BAD:
        return "WIFI_REASON_DISASSOC_PWRCAP_BAD";
    case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
        return "WIFI_REASON_DISASSOC_SUPCHAN_BAD";
    case WIFI_REASON_BSS_TRANSITION_DISASSOC:
        return "WIFI_REASON_BSS_TRANSITION_DISASSOC";
    case WIFI_REASON_IE_INVALID:
        return "WIFI_REASON_IE_INVALID";
    case WIFI_REASON_MIC_FAILURE:
        return "WIFI_REASON_MIC_FAILURE";
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
        return "WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
        return "WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT";
    case WIFI_REASON_IE_IN_4WAY_DIFFERS:
        return "WIFI_REASON_IE_IN_4WAY_DIFFERS";
    case WIFI_REASON_GROUP_CIPHER_INVALID:
        return "WIFI_REASON_GROUP_CIPHER_INVALID";
    case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
        return "WIFI_REASON_PAIRWISE_CIPHER_INVALID";
    case WIFI_REASON_AKMP_INVALID:
        return "WIFI_REASON_AKMP_INVALID";
    case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
        return "WIFI_REASON_UNSUPP_RSN_IE_VERSION";
    case WIFI_REASON_INVALID_RSN_IE_CAP:
        return "WIFI_REASON_INVALID_RSN_IE_CAP";
    case WIFI_REASON_802_1X_AUTH_FAILED:
        return "WIFI_REASON_802_1X_AUTH_FAILED";
    case WIFI_REASON_CIPHER_SUITE_REJECTED:
        return "WIFI_REASON_CIPHER_SUITE_REJECTED";
    case WIFI_REASON_TDLS_PEER_UNREACHABLE:
        return "WIFI_REASON_TDLS_PEER_UNREACHABLE";
    case WIFI_REASON_TDLS_UNSPECIFIED:
        return "WIFI_REASON_TDLS_UNSPECIFIED";
    case WIFI_REASON_SSP_REQUESTED_DISASSOC:
        return "WIFI_REASON_SSP_REQUESTED_DISASSOC";
    case WIFI_REASON_NO_SSP_ROAMING_AGREEMENT:
        return "WIFI_REASON_NO_SSP_ROAMING_AGREEMENT";
    case WIFI_REASON_BAD_CIPHER_OR_AKM:
        return "WIFI_REASON_BAD_CIPHER_OR_AKM";
    case WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION:
        return "WIFI_REASON_NOT_AUTHORIZED_THIS_LOCATION";
    case WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS:
        return "WIFI_REASON_SERVICE_CHANGE_PERCLUDES_TS";
    case WIFI_REASON_UNSPECIFIED_QOS:
        return "WIFI_REASON_UNSPECIFIED_QOS";
    case WIFI_REASON_NOT_ENOUGH_BANDWIDTH:
        return "WIFI_REASON_NOT_ENOUGH_BANDWIDTH";
    case WIFI_REASON_MISSING_ACKS:
        return "WIFI_REASON_MISSING_ACKS";
    case WIFI_REASON_EXCEEDED_TXOP:
        return "WIFI_REASON_EXCEEDED_TXOP";
    case WIFI_REASON_STA_LEAVING:
        return "WIFI_REASON_STA_LEAVING";
    case WIFI_REASON_END_BA:
        return "WIFI_REASON_END_BA";
    case WIFI_REASON_UNKNOWN_BA:
        return "WIFI_REASON_UNKNOWN_BA";
    case WIFI_REASON_TIMEOUT:
        return "WIFI_REASON_TIMEOUT";
    case WIFI_REASON_PEER_INITIATED:
        return "WIFI_REASON_PEER_INITIATED";
    case WIFI_REASON_AP_INITIATED:
        return "WIFI_REASON_AP_INITIATED";
    case WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT:
        return "WIFI_REASON_INVALID_FT_ACTION_FRAME_COUNT";
    case WIFI_REASON_INVALID_PMKID:
        return "WIFI_REASON_INVALID_PMKID";
    case WIFI_REASON_INVALID_MDE:
        return "WIFI_REASON_INVALID_MDE";
    case WIFI_REASON_INVALID_FTE:
        return "WIFI_REASON_INVALID_FTE";
    case WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED:
        return "WIFI_REASON_TRANSMISSION_LINK_ESTABLISH_FAILED";
    case WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED:
        return "WIFI_REASON_ALTERATIVE_CHANNEL_OCCUPIED";
    case WIFI_REASON_BEACON_TIMEOUT:
        return "WIFI_REASON_BEACON_TIMEOUT";
    case WIFI_REASON_NO_AP_FOUND:
        return "WIFI_REASON_NO_AP_FOUND";
    case WIFI_REASON_AUTH_FAIL:
        return "WIFI_REASON_AUTH_FAIL";
    case WIFI_REASON_ASSOC_FAIL:
        return "WIFI_REASON_ASSOC_FAIL";
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
        return "WIFI_REASON_HANDSHAKE_TIMEOUT";
    case WIFI_REASON_CONNECTION_FAIL:
        return "WIFI_REASON_CONNECTION_FAIL";
    case WIFI_REASON_AP_TSF_RESET:
        return "WIFI_REASON_AP_TSF_RESET";
    case WIFI_REASON_ROAMING:
        return "WIFI_REASON_ROAMING";
    case WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG:
        return "WIFI_REASON_ASSOC_COMEBACK_TIME_TOO_LONG";
    case WIFI_REASON_SA_QUERY_TIMEOUT:
        return "WIFI_REASON_SA_QUERY_TIMEOUT";

    default:
        return "UNKNOWN";
    }
}

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{

    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START");
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
        wifi_retry_count = 0;

        break;
    case WIFI_EVENT_STA_DISCONNECTED:
    {
        wifi_event_sta_disconnected_t *wifi_event_sta_disconnected = event_data;

        ESP_LOGW(TAG, "WIFI_EVENT_STA_DISCONNECTED: Error Code (%d), Reason - %s", wifi_event_sta_disconnected->reason, get_wifi_disconnection_str(wifi_event_sta_disconnected->reason));
        if (wifi_event_sta_disconnected->reason == WIFI_REASON_NO_AP_FOUND || wifi_event_sta_disconnected->reason == WIFI_REASON_ASSOC_LEAVE || wifi_event_sta_disconnected->reason == WIFI_REASON_AUTH_EXPIRE)
        {
            if (wifi_retry_count++ < 5)
            {
                ESP_LOGI(TAG, "Retrying to connect");
                vTaskDelay(pdMS_TO_TICKS(5000));
                esp_wifi_connect();
                break;
            }
        }
        xEventGroupSetBits(wifi_events, DISCONNECT);
        set_event_bit(WIFI_DISCONNECTED_BIT);

        break;
    }

    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        xEventGroupSetBits(wifi_events, CONNECTED);

        // Stop the WiFi connection timer as connection was successful
        if (wifi_connection_timer != NULL)
        {
            xTimerStop(wifi_connection_timer, 0);
        }

        if (wifi_connected_event_cb)
        {
            wifi_app_call_callback();
        }

        break;
    default:
        break;
    }
}

void wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Create the WiFi connection timer (1 minute)
    wifi_connection_timer = xTimerCreate(
        "WiFi Conn Timer",             // Timer name
        pdMS_TO_TICKS(60000),          // 1 minute in ticks
        pdFALSE,                       // One-shot timer
        (void *)0,                     // Timer ID
        wifi_connection_timer_callback // Callback function
    );

    if (wifi_connection_timer == NULL)
    {
        ESP_LOGE("Timer", "Failed to create WiFi connection timer");
    }
    else
    {
        ESP_LOGI("Timer", "WiFi connection timer created successfully");
    }
}

void wifi_app_set_callback(wifi_connected_event_callback_t cb)
{
    wifi_connected_event_cb = cb;
}

void wifi_app_call_callback(void)
{
    wifi_connected_event_cb();
}
esp_err_t wifi_connect_sta()
{
    wifi_events = xEventGroupCreate();
    char ssid[33]; // 32 characters + 1 for the null terminator
    char pass[65]; // 64 characters + 1 for the null terminator

    esp_netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t wifi_config = {};
    esp_err_t err = app_nvs_retrieve_sta_creds(ssid, sizeof(ssid), pass, sizeof(pass));
    if (err != ESP_OK)
    {
        set_event_bit(WIFI_DISCONNECTED_BIT);
        return err;
    }
    // Copy SSID and password into the Wi-Fi configuration structure
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0'; // Ensure null-termination
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0'; // Ensure null-termination

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    // Start the WiFi connection timer
    if (wifi_connection_timer != NULL)
    {
        if (xTimerStart(wifi_connection_timer, 0) != pdPASS)
        {
            ESP_LOGE("Timer", "Failed to start WiFi connection timer");
        }
        else
        {
            ESP_LOGI("Timer", "WiFi connection timer started");
        }
    }

    // Wait for either CONNECTED or DISCONNECT event with a timeout
    EventBits_t event_bits = xEventGroupWaitBits(wifi_events, (CONNECTED | DISCONNECT), true, false, pdMS_TO_TICKS(300000 + 1000)); // 5 minutes + 1 sec

    if (event_bits & CONNECTED)
    {
        // Stop the timer as connection was successful
        if (wifi_connection_timer != NULL)
        {
            if (xTimerStop(wifi_connection_timer, 0) != pdPASS)
            {
                ESP_LOGE("Timer", "Failed to stop WiFi connection timer");
            }
            else
            {
                ESP_LOGI("Timer", "WiFi connection timer stopped");
            }
        }
        return ESP_OK;
    }
    else if (event_bits & DISCONNECT)
    {
        // Stop the timer as connection failed
        if (wifi_connection_timer != NULL)
        {
            if (xTimerStop(wifi_connection_timer, 0) != pdPASS)
            {
                ESP_LOGE("Timer", "Failed to stop WiFi connection timer");
            }
            else
            {
                ESP_LOGI("Timer", "WiFi connection timer stopped");
            }
        }
        return ESP_FAIL;
    }
    else
    {
        // Unexpected event bits, stop the timer
        if (wifi_connection_timer != NULL)
        {
            if (xTimerStop(wifi_connection_timer, 0) != pdPASS)
            {
                ESP_LOGE("Timer", "Failed to stop WiFi connection timer");
            }
            else
            {
                ESP_LOGI("Timer", "WiFi connection timer stopped");
            }
        }
        ESP_LOGW(TAG, "WiFi connection wait timed out");
        return ESP_FAIL;
    }
}
void wifi_app_connected_events(void)
{
    ESP_LOGI(TAG, "WiFi Application Connected!!");

    aws_iot_start();
    // Here we place items when want executed when the wifi connects
}

void wifi_connection_timer_callback(TimerHandle_t xTimer)
{
    ESP_LOGI("Timer", "WiFi connection timeout reached. Initiating sleep mode.");

    start_sleep_ble_wifi();
    if (clear_nvs_data() != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear NVS data");
    }
}