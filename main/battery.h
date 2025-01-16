#ifndef BATTERY_H_
#define BATTERY_H_

#define MAX_VOLTAGE 2.1
#define MIN_VOLTAGE 1.4

extern int battery_percentage;
/**
 * Convert battery voltage to percentage
 */
void read_battery_level(void);
/**
 * Function to return battery percentage
 * @return battery percentage
 */
int get_battery_percentage(void);

#endif
