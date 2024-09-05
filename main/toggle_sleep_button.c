#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "tasks_common.h"
#include "toggle_sleep_button.h"

static const char TAG[] = "toggle_sleep_button";
// sleep for _ seconds (in µs)
uint64_t sleep_time = 40000000;
// Semaphore handle
SemaphoreHandle_t toggle_sleep_semaphore = NULL;

/**
 * ISR handler for toggle sleep button
 * @param arg parameter which can be passed to the ISR handler
 */
void IRAM_ATTR toggle_reset_button_isr_handler(void *arg)
{
    // Notify the button task
    xSemaphoreGiveFromISR(toggle_sleep_semaphore, NULL);
}

/**
 * Toggle sleep button task reacts to an external button event by placing device into light sleep
 * @param pvParam parameter which can be passed to the task
 */
void toggle_sleep_button_task(void *pvParam)
{
    for (;;)
    {
        if (xSemaphoreTake(toggle_sleep_semaphore, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG, "TOGGLE DEEP SLEEP INTERRUPT OCCURED");

            // Start sleep mode
            esp_wifi_stop(); // stop wifi

            // Check if button is still pressed (debounce)
            vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay for debounce
            if (gpio_get_level(TOGGLE_SLEEP_BUTTON) == 0)
            {

                while (gpio_get_level(TOGGLE_SLEEP_BUTTON) == 0)
                {
                    vTaskDelay(pdMS_TO_TICKS(10)); // wait for button release
                }
            }

            // if (esp_sleep_is_valid_wakeup_gpio(TOGGLE_SLEEP_BUTTON))
            // {
            //     printf("GPIO is valid for sleep\n");
            //     esp_sleep_enable_ext0_wakeup(TOGGLE_SLEEP_BUTTON, 0); // enable gpio wakeup after enabling sleep timer
            // }
            // else
            // {
            //     printf("GPIO is invalid for sleep!!!\n");
            // }
            ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(sleep_time)); // enable wake up source

            ESP_ERROR_CHECK(esp_deep_sleep_try_to_start()); // returns if sleep is rejected
        }
    }
}
void sleep_toggle_button_config(void)
{
    // Create the binary semaphore
    toggle_sleep_semaphore = xSemaphoreCreateBinary();

    // configure the button and set the direction
    esp_rom_gpio_pad_select_gpio(TOGGLE_SLEEP_BUTTON);
    gpio_set_direction(TOGGLE_SLEEP_BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(TOGGLE_SLEEP_BUTTON, GPIO_PULLUP_ONLY);

    // Enable interrupt on the negative edge
    gpio_set_intr_type(TOGGLE_SLEEP_BUTTON, GPIO_INTR_NEGEDGE);

    // create the timer sleep task
    xTaskCreatePinnedToCore(&toggle_sleep_button_task, "toggle_sleep_button_task", SLEEP_TOGGLE_BUTTON_TASK_STACK_SIZE, NULL, SLEEP_TOGGLE_BUTTON_TASK_PRIORITY, NULL, SLEEP_TOGGLE_BUTTON_TASK_CORE_ID);

    // Install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    // Attach the interrup service routine
    gpio_isr_handler_add(TOGGLE_SLEEP_BUTTON, toggle_reset_button_isr_handler, NULL);
}

void start_sleep(void)
{
    // Start sleep mode

    if (esp_sleep_is_valid_wakeup_gpio(TOGGLE_SLEEP_BUTTON))
    {
        printf("GPIO is valid for sleep\n");
        esp_sleep_enable_ext0_wakeup(TOGGLE_SLEEP_BUTTON, 0); //>enable gpio wakeup after enabling sleep timer
                                                              //> returns if sleep is rejected
    }
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(sleep_time)); //> enable wake up source

    printf("Attempting to place device in light sleep, wake up source is timer and gpio\n");
    ESP_ERROR_CHECK(esp_deep_sleep_try_to_start());
}