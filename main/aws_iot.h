/*
 * aws_iot.h
 *
 *  Created on: Mar 13, 2022
 *      Author: kjagu
 */

#ifndef MAIN_AWS_IOT_H_
#define MAIN_AWS_IOT_H_

#define CONFIG_AWS_EXAMPLE_CLIENT_ID "Plant_Monitor"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/**
 * Starts AWS IoT task.
 */
void aws_iot_start(void);

/**
 *
 */
static SemaphoreHandle_t wifi_callback_semaphore;

#endif /* MAIN_AWS_IOT_H_ */
