#ifndef MOISTURE_SENSOR_H_
#define MOISTURE_SENSOR_H_
#include "driver/adc.h"

#define MOISTURE_SENSOR_OK 0
#define MOISTURE_SENSOR_TIMEOUT_ERROR
#define MOISTURE_SENSOR_DEFAULT_VREF 1100
/**
 * Starts the moisture sensor task
 */
void moisture_sensor_task_start(void);

/**
 * Reads raw moisture sensor data and converts to moisture level
 * @return status
 */
int read_moisture_sensor(void);
/**
 * function to return moisture reading
 * @return moisture
 */
float get_moisture(void);
/**
 * Error handler checks return of read moisture sensor function and logs error
 * @params takes response from read moisture sensor
 */
void error_handler(int res);
#endif