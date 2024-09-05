#include "global_event_group.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "toggle_sleep_button.h"
#include "esp_err.h"
#include "esp_log.h"

// Define the event group handle
EventGroupHandle_t app_event_group;

void init_event_manager(void)
{
    // Create the event group
    app_event_group = xEventGroupCreate();
    if (app_event_group == NULL)
    {
        ESP_LOGE("EVENT_MANAGER", "Failed to create event group");
    }
}

void set_event_bit(EventBits_t bit)
{
    xEventGroupSetBits(app_event_group, bit);
}

void handle_event_bits(void *param)
{
    while (1)
    {
        EventBits_t bits = xEventGroupWaitBits(app_event_group,
                                               AWS_IOT_SUCCESS_BIT | WIFI_DISCONNECTED_BIT,
                                               pdTRUE,  // Clear the bits on exit
                                               pdFALSE, // Wait for any bit to be set
                                               portMAX_DELAY);

        if (bits & AWS_IOT_SUCCESS_BIT)
        {
            ESP_LOGI("EVENT_MANAGER", "AWS IoT transmission successful!");
            start_sleep();
        }

        if (bits & WIFI_DISCONNECTED_BIT)
        {
            ESP_LOGI("EVENT_MANAGER", "Wi-Fi disconnected!");
            // Perform actions related to Wi-Fi disconnection
            start_sleep();
        }
    }
}
