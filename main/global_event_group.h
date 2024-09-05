#ifndef GLOBAL_EVENT_GROUP_H
#define GLOBAL_EVENT_GROUP_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

// Define event bits
#define AWS_IOT_SUCCESS_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1

// Declare the event group handle
extern EventGroupHandle_t app_event_group;

// Function to initialize the event group
void init_event_manager(void);

// Function to set event bits
void set_event_bit(EventBits_t bit);

// Function to wait for event bits and handle them
void handle_event_bits(void *param);
#endif
