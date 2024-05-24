
#include <Arduino.h>
#include "led.h"
#include "constants.h"


/********** Constructor **********/
// Led::Led() {}


/********** Config and init LED **********/
void ledInit()
{
    pinMode(WATERING_LED, OUTPUT);
    pinMode(OVERFLOW_LED, OUTPUT);

    digitalWrite(WATERING_LED, 1);
    digitalWrite(OVERFLOW_LED, 1);
}

void ledWrite(uint8_t pin, uint8_t value)
{
    digitalWrite(pin, value);
}