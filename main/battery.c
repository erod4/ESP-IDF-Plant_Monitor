#include "battery.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"

int battery_percentage = 0;

void read_battery_level(void)
{
    // Read battery voltage level
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
    uint32_t adc_reading = adc1_get_raw(ADC1_CHANNEL_7);

    // Convert battery voltage to percentage
    float voltage = adc_reading * (3.3 / 4095.0);
    battery_percentage = (voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE) * 100;
}

int get_battery_percentage(void)
{

    return battery_percentage;
}
