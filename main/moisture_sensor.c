
#include "moisture_sensor.h"

#include <stdio.h>

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tasks_common.h"

// static const char* TAG = "Moisture Sensor";
float voltage = 0.0;

int read_moisture_sensor(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

    uint32_t adc_reading = adc1_get_raw(ADC1_CHANNEL_6);

    voltage = adc_reading * (3.3 / 4095.0);

    // // Print the voltage
    // printf("Raw: %lu\tVoltage: %fmV\n", adc_reading, voltage);

    return MOISTURE_SENSOR_OK;
}
float get_moisture() { return voltage; }

void error_handler(int res)
{
    switch (res)
    {
    case MOISTURE_SENSOR_OK:
        break;
    //>Note: add more sensor potential errors
    default:
        break;
    }
}
/**
 * Moisture Sensor task
 */
static void moisture_sensor_task(void *pvParameter)
{
    printf("Starting Moisture Sensor Task\n\n");
    for (;;)
    {
        // printf("===Reading Moisture Sensor===\n");
        int ret = read_moisture_sensor();
        error_handler(ret);
        // printf("Moisture: %.1f\n", get_moisture());

        // wait 2 seconds before reading again
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void moisture_sensor_task_start(void)
{
    xTaskCreatePinnedToCore(&moisture_sensor_task, "Moisture_sensor_task", MOISTURE_SENSOR_TASK_STACK_SIZE, NULL, MOISTURE_SENSOR_TASK_PRIORITY, NULL, MOISTURE_SENSOR_TASK_CORE_ID);
}