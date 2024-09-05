#include "AHT20.h"

#include <aht.h>
#include "esp_err.h"
#include "esp_log.h"

#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#include "tasks_common.h"
#ifdef CONFIG_EXAMPLE_I2C_ADDRESS_GND
#define ADDR AHT_I2C_ADDRESS_GND
#endif
#ifdef CONFIG_EXAMPLE_I2C_ADDRESS_VCC
#define ADDR AHT_I2C_ADDRESS_VCC
#endif

#ifdef CONFIG_EXAMPLE_TYPE_AHT1x
#define AHT_TYPE AHT_TYPE_AHT1x
#endif

#ifdef CONFIG_EXAMPLE_TYPE_AHT20
#define AHT_TYPE AHT_TYPE_AHT20
#endif

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif
float temp = 0.0, hum = 0.0;

float getTemp(void)
{
    return temp;
}
float getHum(void)
{
    return hum;
}

static const char *TAG = "aht-example";
/**
 * Humidity & Temperature Sensor task
 */
void read_hum_temp_sensor(void)
{
    aht_t dev = {0};
    dev.mode = AHT_MODE_NORMAL;
    dev.type = AHT_TYPE_AHT20;

    ESP_ERROR_CHECK(aht_init_desc(&dev, AHT_I2C_ADDRESS_GND, 0, I2C_MASTER_SDA, I2C_MASTER_SCL));
    ESP_ERROR_CHECK(aht_init(&dev));

    bool calibrated;
    ESP_ERROR_CHECK(aht_get_status(&dev, NULL, &calibrated));
    if (calibrated)
        ESP_LOGI(TAG, "Sensor calibrated");
    else
        ESP_LOGW(TAG, "Sensor not calibrated!");

    esp_err_t res = aht_get_data(&dev, &temp, &hum);
    if (res == ESP_OK)
        ESP_LOGI(TAG, "Temperature: %.1f°C, Humidity: %.2f%%", temp, hum);
    else
        ESP_LOGE(TAG, "Error reading data: %d (%s)", res, esp_err_to_name(res));
}
