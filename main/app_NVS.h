#ifndef APP_NVS_H_
#define APP_NVS_H_
#include "esp_err.h"

/**
 * Stores network credentials received from mobile app
 * @param ssid, password of network to connect to
 * @return ESP_OK if succesfull
 */
esp_err_t app_nvs_save_sta_creds(char *ssid, char *pass, char *id);
/**
 * Retrieves network credentials received from mobile app and fills buffers
 * @param ssid buffer of network ssid to connect to
 * @param pass buffer of network pass to connect to
 * @return ESP_OK if succesfull
 */
esp_err_t app_nvs_retrieve_sta_creds(char *ssid, size_t ssid_len, char *pass, size_t pass_len);

/**
 * Retrieves user id from NVS
 * @param user_id buffer to store user id
 * @param user_id_len length of user_id buffer
 * @return ESP_OK if succesfull
 */
esp_err_t app_nvs_retrieve_user_id(char *user_id, size_t user_id_len);

/**
 * Clears NVS data
 * @return ESP_OK if succesfull
 */
esp_err_t clear_nvs_data(void);
#endif
