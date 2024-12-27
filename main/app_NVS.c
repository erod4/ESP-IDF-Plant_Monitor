#include "app_NVS.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_mac.h"
const char app_nvs_sta_creds_namespace[] = "STA_CREDS";
static const char TAG[] = "NVS";
esp_err_t app_nvs_save_sta_creds(char *ssid, char *pass, char *id)
{
    esp_err_t esp_err;
    nvs_handle handle;
    esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_save_sta_creds: Error (%s) opening NVS!\n", esp_err_to_name(esp_err));
        return esp_err;
    }
    esp_err = nvs_set_str(handle, "user_id", (void *)id);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_save_sta_creds: Error (%s) saving user id to NVS!\n", esp_err_to_name(esp_err));
        return esp_err;
    }
    esp_err = nvs_set_str(handle, "ssid", (void *)ssid);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_save_sta_creds: Error (%s) saving SSID to NVS!\n", esp_err_to_name(esp_err));
        return esp_err;
    }

    esp_err = nvs_set_str(handle, "pass", (void *)pass);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_save_sta_creds: Error (%s) saving password to NVS!\n", esp_err_to_name(esp_err));
        return esp_err;
    }
    esp_err = nvs_commit(handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_save_sta_creds: Error (%s) commiting credentials to NVS!\n", esp_err_to_name(esp_err));
        return esp_err;
    }
    nvs_close(handle);

    ESP_LOGI(TAG, "app_nvs_save_sta_creds: wrote wifi_sta_config: Station SSID: %s Password: %s", ssid, pass);

    return ESP_OK;
}
esp_err_t app_nvs_retrieve_sta_creds(char *ssid, size_t ssid_len, char *pass, size_t pass_len)
{
    esp_err_t esp_err;
    nvs_handle handle;

    esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READONLY, &handle);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_retrieve_sta_creds: Error (%s) opening NVS!\n", esp_err_to_name(esp_err));
        return esp_err;
    }

    // Retrieve the ssid
    size_t required_size;
    esp_err = nvs_get_str(handle, "ssid", NULL, &required_size);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_retrieve_sta_creds: Error (%s) getting SSID size!\n", esp_err_to_name(esp_err));
        nvs_close(handle);
        return esp_err;
    }

    if (required_size > ssid_len)
    {
        printf("app_nvs_retrieve_sta_creds: SSID buffer too small!\n");
        nvs_close(handle);
        return ESP_ERR_NVS_INVALID_LENGTH;
    }

    esp_err = nvs_get_str(handle, "ssid", ssid, &required_size);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_retrieve_sta_creds: Error (%s) retrieving SSID!\n", esp_err_to_name(esp_err));
        nvs_close(handle);
        return esp_err;
    }

    // Retrieve the pass
    esp_err = nvs_get_str(handle, "pass", NULL, &required_size);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_retrieve_sta_creds: Error (%s) getting password size!\n", esp_err_to_name(esp_err));
        nvs_close(handle);
        return esp_err;
    }

    if (required_size > pass_len)
    {
        printf("app_nvs_retrieve_sta_creds: Password buffer too small!\n");
        nvs_close(handle);
        return ESP_ERR_NVS_INVALID_LENGTH;
    }

    esp_err = nvs_get_str(handle, "pass", pass, &required_size);
    if (esp_err != ESP_OK)
    {
        printf("app_nvs_retrieve_sta_creds: Error (%s) retrieving password!\n", esp_err_to_name(esp_err));
        nvs_close(handle);
        return esp_err;
    }

    nvs_close(handle);

    ESP_LOGI(TAG, "app_nvs_retrieve_sta_creds: Retrieved SSID: %s Password: %s", ssid, pass);

    return ESP_OK;
}

esp_err_t app_nvs_retrieve_user_id(char *user_id, size_t user_id_len)
{
    esp_err_t esp_err;
    nvs_handle handle;

    // Open NVS in read-only mode
    esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READONLY, &handle);
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(esp_err));
        return esp_err;
    }

    // Retrieve the user_id
    size_t required_size;
    esp_err = nvs_get_str(handle, "user_id", NULL, &required_size);
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error getting user_id size: %s", esp_err_to_name(esp_err));
        nvs_close(handle);
        return esp_err;
    }

    if (required_size > user_id_len)
    {
        ESP_LOGE(TAG, "User_id buffer too small!");
        nvs_close(handle);
        return ESP_ERR_NVS_INVALID_LENGTH;
    }

    esp_err = nvs_get_str(handle, "user_id", user_id, &required_size);
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error retrieving user_id: %s", esp_err_to_name(esp_err));
        nvs_close(handle);
        return esp_err;
    }

    nvs_close(handle);
    ESP_LOGI(TAG, "Retrieved user_id: %s", user_id);
    return ESP_OK;
}

esp_err_t clear_nvs_data(void)
{
    nvs_handle handle;
    esp_err_t esp_err = nvs_open(app_nvs_sta_creds_namespace, NVS_READWRITE, &handle);
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error opening NVS: %s", esp_err_to_name(esp_err));
        return esp_err;
    }

    // Erase all keys in this namespace
    esp_err = nvs_erase_all(handle);
    if (esp_err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error erasing NVS: %s", esp_err_to_name(esp_err));
        nvs_close(handle);
        return esp_err;
    }

    // Commit the changes
    esp_err = nvs_commit(handle);
    nvs_close(handle);

    if (esp_err == ESP_OK)
    {
        ESP_LOGI(TAG, "NVS credentials and user_id cleared successfully.");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to commit changes after erasing NVS: %s", esp_err_to_name(esp_err));
    }

    return esp_err;
}
