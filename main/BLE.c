#include "BLE.h"
#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "rgb_led.h"
#include "app_NVS.h"
uint8_t ble_addr_type;

static int device_read(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "E.R", strlen("E.R"));
    return 0;
}

void parse_and_save_wifi_credentials(const char *input)
{
    esp_err_t esp_err;
    char id[20];               // Buffer for ID
    char network_id[20];       // Buffer for network ID
    char network_password[50]; // Buffer for network password

    // Split the string using '?' as the delimiter
    char *token = strtok((char *)input, "?");

    // Copy the first part to id
    if (token != NULL)
    {
        strncpy(id, token, sizeof(id));
        id[sizeof(id) - 1] = '\0'; // Ensure null termination
    }

    // Copy the second part to network_id
    token = strtok(NULL, "?");
    if (token != NULL)
    {
        strncpy(network_id, token, sizeof(network_id));
        network_id[sizeof(network_id) - 1] = '\0'; // Ensure null termination
    }

    // Copy the third part to network_password
    token = strtok(NULL, "?");
    if (token != NULL)
    {
        strncpy(network_password, token, sizeof(network_password));
        network_password[sizeof(network_password) - 1] = '\0'; // Ensure null termination
    }

    // Log the parsed data for debugging
    ESP_LOGI("WiFi Credentials", "ID: %s, Network ID: %s, Network Password: %s", id, network_id, network_password);

    // Pass the parsed credentials to your NVS save function
    esp_err = app_nvs_save_sta_creds(network_id, network_password, id);
    if (esp_err == ESP_OK)
    {
        rgb_led_network_credentials_set();
    }
}

// Update the device_write function to use the parsing function
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    char input_data[100]; // Buffer to hold the received string

    // Copy received data into the buffer
    snprintf(input_data, sizeof(input_data), "%.*s", ctxt->om->om_len, ctxt->om->om_data);

    // Parse and save the WiFi credentials
    parse_and_save_wifi_credentials(input_data);

    return 0;
}
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(0x180),
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(0xFEF4),
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_read},
         {.uuid = BLE_UUID16_DECLARE(0xDEAD),
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};

static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_CONNECT %s", event->connect.status == 0 ? "OK" : "FAILED");
        if (event->connect.status == 0)
        {

            ble_app_advertise();
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_DISCONNECT");
        ble_app_advertise();

        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_ADV_COMPLETE");
        ble_app_advertise();

        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI("GAP", "BLE_GAP_EVENT_SUBSCRIBE");
        break;
    default:
        break;
    }
    return 0;
}

void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_DISC_LTD;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen(ble_svc_gap_device_name());
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}
void ble_app_on_sync(void)
{
    ble_hs_id_infer_auto(0, &ble_addr_type);
    ble_app_advertise();
}
void ble_task(void *param)
{
    nimble_port_run();
}
void ble_start(void)
{
    ESP_ERROR_CHECK(nimble_port_init());
    ESP_ERROR_CHECK(ble_svc_gap_device_name_set(DEVICE_NAME));
    ble_svc_gap_init();
    ble_svc_gatt_init();

    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);

    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(ble_task);
    rgb_led_ble_advertising();
}