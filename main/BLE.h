#ifndef BLE_H
#define BLE_H
#define DEVICE_NAME "Plant Monitor"
#define DEVICE_WIFI_SERVICE 0x180A
#define MANUFACTURER_NAME 0x2A29

#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
/**
 * Inititializes Nimble BLE
 */
void ble_start(void);

/**
 * Starts BLE advertizing
 */
void ble_app_advertise(void);

void ble_adv_timer_callback(TimerHandle_t xTimer);

#endif
