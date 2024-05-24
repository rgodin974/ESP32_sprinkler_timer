
#include <Arduino.h>

#include "config.h"
#include "sensors.h"

// Humidity sensor
int getSoilMoisture(int *value)
{
    uint8_t sensor = Config::getConfig()->getMoistureSensor();
    if (sensor != 0) {
        uint8_t maxMoisture = Config::getConfig()->getMaxMoisture();
        unsigned percent = 100 - maxMoisture;       // Reverse percentage
        unsigned intervals = (HUMIDITY_AIR - HUMIDITY_WATER) / (100 / percent);
        unsigned limit = HUMIDITY_WATER + intervals;
        analogSetPinAttenuation(sensor, ADC_0db);   // See if it's necessary
        unsigned soilMoistureValue = analogRead(sensor);
        soilMoistureValue = soilMoistureValue < HUMIDITY_WATER ? HUMIDITY_WATER : soilMoistureValue;
        soilMoistureValue = soilMoistureValue > HUMIDITY_AIR ? HUMIDITY_AIR : soilMoistureValue;
        unsigned maxDiff = HUMIDITY_AIR - HUMIDITY_WATER;
        unsigned tmp = HUMIDITY_AIR - soilMoistureValue;
        unsigned diff = maxDiff - tmp;
        *value = 100 - (diff * 100 / maxDiff);
            
        if (soilMoistureValue < (limit)) {
            // DEBUG Serial.printf("Humidity: %d (limit %d), WET\n", soilMoistureValue, limit);
            return HUMIDITY_WET;
        }
        else {
            // DEBUG Serial.printf("Humidity: %d (limit %d), DRY\n", soilMoistureValue, limit);
            return HUMIDITY_DRY;
        }
    }
    Serial.printf("Humidity: no sensor, DRY\n");
    *value = 50;
    return HUMIDITY_DRY;
}


// Flow sensor
#define SENSOR_INTERRUPT digitalPinToInterrupt(FLOW_SENSOR)

float calibrationFactor = 1.08;

static volatile byte pulseCount;
static unsigned long total;

unsigned long oldTime;

void IRAM_ATTR pulseCounter()
{
    pulseCount++;
}

void flowInit(void)
{
    uint8_t sensor = Config::getConfig()->getFlowSensor();
    if (sensor != 0) {
        pinMode(sensor, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(sensor), pulseCounter, FALLING);
    }
}

float getFlow(void)
{
    float flowRate = 0;
    unsigned int milliLiters;

    if (Config::getConfig()->getFlowSensor() == 0) {
        return 0.0;
    }
    if (millis() - oldTime == 0) {
        return 0;
    }
    noInterrupts();
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    oldTime = millis();
    milliLiters = (flowRate / 60) * 1000;
    total += milliLiters;
    pulseCount = 0;
    interrupts();
    // DEBUG Serial.printf("flow: %f\n", flowRate);
    return flowRate;
}