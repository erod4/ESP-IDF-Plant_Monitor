#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

// RGB LED GPIO
#define RGB_LED_RED_GPIO 26   // Prev 27
#define RGB_LED_GREEN_GPIO 27 // Prev 26
#define RGB_LED_BLUE_GPIO 25  // Don't Touch

// RGB lED color mix channels
#define RGB_LED_CHANNEL_NUM 3

// RGB LED config
typedef struct
{
    int channel;
    int gpio;
    int mode;
    int timer_index;
} ledc_info_t;
extern ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM];

/**
 * Color to indicate WIFI application has started
 * */

void rgb_led_wifi_disconnected(void);

/**
 * Color to indicate HTTP server has started
 * */
void rgb_led_http_server_started(void);

/**
 * Color to indicate ESP32 is connected to access point
 * */

void rgb_led_wifi_connected(void);

/**
 * Produces blue indicator for ble advertising
 */
void rgb_led_ble_advertising(void);
/**
 * Produces a green indicator when network credentials have been set
 */
void rgb_led_network_credentials_set(void);
#endif