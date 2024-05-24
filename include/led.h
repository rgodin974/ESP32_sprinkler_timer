
#ifndef _LED_H_
#define _LED_H_

// Define LED pin
#define OVERFLOW_LED    32
#define WATERING_LED    33

void ledInit();
void ledWrite(uint8_t pin, uint8_t value);

#endif