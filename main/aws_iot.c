/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * Additions Copyright 2016 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
/**
 * @file subscribe_publish_sample.c
 * @brief simple MQTT publish and subscribe on the same topic
 *
 * This example takes the parameters from the build configuration and establishes a connection to the AWS IoT MQTT Platform.
 * It subscribes and publishes to the same topic - "test_topic/esp32"
 *
 * Some setup is required. See example README for details.
 *
 */
#include "aws_iot.h"
#include "app_NVS.h"
#include "cJSON.h"

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "AHT20.h"
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_version.h"

#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "moisture_sensor.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "tasks_common.h"
#include "wifi_app.h"
#include "global_event_group.h"

static const char *TAG = "aws_iot";
static const char *ID = "507f1f77bcf86cd799439011";
// AWS IoT task handle
static TaskHandle_t task_aws_iot = NULL;

/**
 * CA Root certificate, device ("Thing") certificate and device ("Thing") key.
 * "Embedded Certs" are loaded from files in "certs/" and embedded into the app binary.
 */
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");

/**
 * @brief Default MQTT HOST URL is pulled from the aws_iot_config.h
 */
char HostAddress[255] = AWS_IOT_MQTT_HOST;

/**
 * @brief Default MQTT port is pulled from the aws_iot_config.h
 */
uint32_t port = AWS_IOT_MQTT_PORT;

void iot_subscribe_callback_handler(AWS_IoT_Client *pClient, char *topicName, uint16_t topicNameLen,
                                    IoT_Publish_Message_Params *params, void *pData)
{
    ESP_LOGI(TAG, "Subscribe callback Test: %.*s\t%.*s", topicNameLen, topicName, (int)params->payloadLen, (char *)params->payload);
    char incoming_msg[128];
    memset(incoming_msg, 0, sizeof(incoming_msg));
    memcpy(incoming_msg, params->payload, params->payloadLen);

    // Check if this message is from the delete topic
    if (strncmp(topicName, "delete", topicNameLen) == 0)
    {
        // Parse JSON
        cJSON *root = cJSON_Parse(incoming_msg);
        if (root == NULL)
        {
            ESP_LOGE(TAG, "Failed to parse JSON");
            return;
        }

        cJSON *msg = cJSON_GetObjectItem(root, "message");
        if (cJSON_IsString(msg) && (msg->valuestring != NULL))
        {
            // Compare extracted message to ID
            if (strcmp(msg->valuestring, ID) == 0)
            {
                ESP_LOGI(TAG, "Delete request matches our user_id. Clearing credentials...");
                if (clear_nvs_data() == ESP_OK)
                {
                    ESP_LOGI(TAG, "Device creds and user_id cleared.");

                    // Publish acknowledgment back to "delete" topic
                    const char *ack_topic = "delete";
                    const char *ack_payload = ""; // Empty payload for acknowledgment

                    IoT_Publish_Message_Params ack_params;
                    ack_params.qos = QOS1; // Use QoS0 for acknowledgment
                    ack_params.payload = (void *)ack_payload;
                    ack_params.payloadLen = strlen(ack_payload);
                    ack_params.isRetained = 1; // Retain the message

                    IoT_Error_t rc = aws_iot_mqtt_publish(pClient, ack_topic, (uint16_t)strlen(ack_topic), &ack_params);
                    if (rc == SUCCESS)
                    {
                        ESP_LOGI(TAG, "Acknowledge message published successfully on the same 'delete' topic.");
                    }
                    else
                    {
                        ESP_LOGE(TAG, "Failed to publish acknowledge message. Error: %d", rc);
                    }
                    set_event_bit(WIFI_DISCONNECTED_BIT);
                }
            }
            else
            {
                ESP_LOGI(TAG, "Delete request received but does not match ID.");
            }
        }
        else
        {
            ESP_LOGI(TAG, "No 'message' field in JSON or it's not a string");
        }

        cJSON_Delete(root);
    }
}
void disconnectCallbackHandler(AWS_IoT_Client *pClient, void *data)
{
    ESP_LOGW(TAG, "MQTT Disconnect");
    IoT_Error_t rc = FAILURE;

    if (NULL == pClient)
    {
        return;
    }

    if (aws_iot_is_autoreconnect_enabled(pClient))
    {
        ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
    }
    else
    {
        ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");
        rc = aws_iot_mqtt_attempt_reconnect(pClient);
        if (NETWORK_RECONNECTED == rc)
        {
            ESP_LOGW(TAG, "Manual Reconnect Successful");
        }
        else
        {
            ESP_LOGW(TAG, "Manual Reconnect Failed - %d", rc);
        }
    }
}

void aws_iot_task(void *param)
{
    char cPayload[512];
    char user_id[64];
    int32_t i = 0;

    IoT_Error_t rc = FAILURE;
    // Retrieve user_id from NVS
    if (app_nvs_retrieve_user_id(user_id, sizeof(user_id)) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to retrieve user_id from NVS");
        // Handle this error as appropriate for your application
        // For now, we will just set a default user_id.
        strcpy(user_id, "unknown");
    }
    AWS_IoT_Client client;
    IoT_Client_Init_Params mqttInitParams = iotClientInitParamsDefault;
    IoT_Client_Connect_Params connectParams = iotClientConnectParamsDefault;

    IoT_Publish_Message_Params paramsQOS0;
    IoT_Publish_Message_Params paramsQOS1;

    ESP_LOGI(TAG, "AWS IoT SDK Version %d.%d.%d-%s", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_TAG);

    mqttInitParams.enableAutoReconnect = false; // We enable this later below
    mqttInitParams.pHostURL = HostAddress;
    mqttInitParams.port = port;

    mqttInitParams.pRootCALocation = (const char *)aws_root_ca_pem_start;
    mqttInitParams.pDeviceCertLocation = (const char *)certificate_pem_crt_start;
    mqttInitParams.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;

    mqttInitParams.mqttCommandTimeout_ms = 20000;
    mqttInitParams.tlsHandshakeTimeout_ms = 5000;
    mqttInitParams.isSSLHostnameVerify = true;
    mqttInitParams.disconnectHandler = disconnectCallbackHandler;
    mqttInitParams.disconnectHandlerData = NULL;

    rc = aws_iot_mqtt_init(&client, &mqttInitParams);
    if (SUCCESS != rc)
    {
        ESP_LOGE(TAG, "aws_iot_mqtt_init returned error : %d ", rc);
        abort();
    }

    connectParams.keepAliveIntervalInSec = 10;
    connectParams.isCleanSession = true;
    connectParams.MQTTVersion = MQTT_3_1_1;
    /* Client ID is set in aws_iot.h and AKA your Thing's Name in AWS IoT */
    connectParams.pClientID = CONFIG_AWS_EXAMPLE_CLIENT_ID;
    connectParams.clientIDLen = (uint16_t)strlen(CONFIG_AWS_EXAMPLE_CLIENT_ID);
    connectParams.isWillMsgPresent = false;

    ESP_LOGI(TAG, "Connecting to AWS...");
    do
    {
        rc = aws_iot_mqtt_connect(&client, &connectParams);
        if (SUCCESS != rc)
        {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, mqttInitParams.pHostURL, mqttInitParams.port);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    } while (SUCCESS != rc);

    /*
     * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
     *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
     *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
     */
    rc = aws_iot_mqtt_autoreconnect_set_status(&client, true);
    if (SUCCESS != rc)
    {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
        abort();
    }

    const char *TOPIC = "test_topic/esp32";
    const int TOPIC_LEN = strlen(TOPIC);
    const char *DELETE_TOPIC = "delete";
    const int DELETE_TOPIC_LEN = strlen(DELETE_TOPIC);

    ESP_LOGI(TAG, "Subscribing...");
    rc = aws_iot_mqtt_subscribe(&client, TOPIC, TOPIC_LEN, QOS1, iot_subscribe_callback_handler, NULL);
    if (SUCCESS != rc)
    {
        ESP_LOGE(TAG, "Error subscribing : %d ", rc);
        abort();
    }
    ESP_LOGI(TAG, "Subscribing to delete topic...");
    rc = aws_iot_mqtt_subscribe(&client, DELETE_TOPIC, DELETE_TOPIC_LEN, QOS1, iot_subscribe_callback_handler, NULL);
    if (SUCCESS != rc)
    {
        ESP_LOGE(TAG, "Error subscribing to delete topic: %d ", rc);
        abort();
    }
    static bool aws_message_sent_once = false;

    // sprintf(cPayload, "%s : %ld ", "hello from SDK", i);

    paramsQOS0.qos = QOS0;
    paramsQOS0.payload = (void *)cPayload;
    paramsQOS0.isRetained = 0;

    paramsQOS1.qos = QOS1;
    paramsQOS1.payload = (void *)cPayload;
    paramsQOS1.isRetained = 0;

    while ((NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc))
    {
        // Max time the yield function will wait for read messages
        rc = aws_iot_mqtt_yield(&client, 100);
        if (NETWORK_ATTEMPTING_RECONNECT == rc)
        {
            // If the client is attempting to reconnect we will skip the rest of the loop.
            continue;
        }

        ESP_LOGI(TAG, "Stack remaining for task '%s' is %d bytes", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL));
        vTaskDelay(3000 / portTICK_PERIOD_MS);

        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS0);
        // sprintf(cPayload, "%s ,%s : %.1f, %s : %.1f, %s : %.1f", "1", "Temperature", getTemp(), "Humidity", getHum(), "Moisture", get_moisture());

        sprintf(cPayload, "%s,%s,%.1f,%.1f,%.1f,9", user_id, ID, getTemp(), getHum(), get_moisture());
        paramsQOS1.payloadLen = strlen(cPayload);
        rc = aws_iot_mqtt_publish(&client, TOPIC, TOPIC_LEN, &paramsQOS1);
        if (rc == SUCCESS)
        {
            // Set the event bit when the message is successfully delivered
            set_event_bit(AWS_IOT_SUCCESS_BIT);
            // abort();
        }
        else if (rc == MQTT_REQUEST_TIMEOUT_ERROR)
        {
            ESP_LOGW(TAG, "QOS0 publish ack not received.");
            rc = SUCCESS;
        }
    }

    ESP_LOGE(TAG, "An error occurred in the main loop.");

    abort();
}

void aws_iot_start(void)
{
    if (task_aws_iot == NULL)
    {
        xTaskCreatePinnedToCore(&aws_iot_task, "aws_iot_task", AWS_IOT_TASK_STACK_SIZE, NULL, AWS_IOT_TASK_PRIORITY, &task_aws_iot, AWS_IOT_TASK_CORE_ID);
    }
}
