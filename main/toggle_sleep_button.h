#ifndef TOGGLE_SLEEP_BUTTON_H_
#define TOGGLE_SLEEP_BUTTON_H_

// Default Interrupt Flag
#define ESP_INTR_FLAG_DEFAULT 0

// toggle sleep button is the
#define TOGGLE_SLEEP_BUTTON 0

/**
 * Configures sleep toggle button and interrup configuration
 */
void sleep_toggle_button_config(void);
/**
 * Configures deep sleep and starts deep with wakeup source timer and boot button
 */
void start_sleep(void);
#endif
