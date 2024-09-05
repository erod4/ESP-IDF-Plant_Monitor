#ifndef BLE_H
#define BLE_H
#define DEVICE_NAME "Plant Monitor"
#define DEVICE_WIFI_SERVICE 0x180A
#define MANUFACTURER_NAME 0x2A29
/**
 * Inititializes Nimble BLE
 */
void ble_start(void);

/**
 * Starts BLE advertizing
 */
void ble_app_advertise(void);

#endif
