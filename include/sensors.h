
#ifndef _SENSORS_H_
#define _SENSORS_H_

#define HUMIDITY_DRY        1
#define HUMIDITY_WET        2

// Moisture sensor
int getSoilMoisture(int *value);

// Flow sensor
void flowInit(void);
float getFlow(void);

#endif
