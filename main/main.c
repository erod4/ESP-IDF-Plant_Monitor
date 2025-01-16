#include <stdio.h>
#include "global_event_group.h"
#include <string.h>
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "BLE.h"
#include "AHT20.h"
#include "WiFi_app.h"
#include "esp_sleep.h"
#include "aws_iot.h"
#include "toggle_sleep_button.h"
#include "moisture_sensor.h"
#include "i2cdev.h"
#include "battery.h"

#define TAG "MAIN"

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(i2cdev_init());

    init_event_manager();
    xTaskCreate(handle_event_bits, "handle_events_task", 4096, NULL, 5, NULL);
    read_moisture_sensor();
    read_hum_temp_sensor();
    read_battery_level();
    // moisture_sensor_task_start();
    // hum_temp_sensor_task_start();

    esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
    switch (wakeup_cause)
    {
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        ESP_LOGI(TAG, "Wakeup cause: Undefined (first boot or reset)");
        ble_start();
        // hum_temp_sensor_task_start();

        sleep_toggle_button_config();

        break;
    case ESP_SLEEP_WAKEUP_EXT0:
        ESP_LOGI(TAG, "Wakeup cause: GPIO (EXT0)");
        ble_start();

        sleep_toggle_button_config();

        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        ESP_LOGI(TAG, "Wakeup cause: GPIO (EXT1)");
        sleep_toggle_button_config();

        break;
    case ESP_SLEEP_WAKEUP_TIMER:

        wifi_app_set_callback(&wifi_app_connected_events);
        ESP_LOGI(TAG, "Wakeup cause: Timer");
        wifi_init();
        wifi_connect_sta();

        break;
    default:
        ESP_LOGI(TAG, "Wakeup cause: Other");
        break;
    }
}
