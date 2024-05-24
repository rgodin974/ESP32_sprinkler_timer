
#include <Arduino.h>
#include <ezButton.h>

#include "hmi.h"
#include "oled.h"
// #include "watering.h"
// #include "humidity.h"
// #include "flow.h"


/********** Constructor **********/
Hmi::Hmi() :
    m_state(IDLE),
    m_manualWatering(0),
    m_time(0)
{
}

ezButton functionButton(FUNCTION_BUTTON);
ezButton manualButton(MANUAL_BUTTON);


/********** Initialization **********/
void Hmi::begin(void)
{
    functionButton.setDebounceTime(50);
    manualButton.setDebounceTime(50);
}


/********** Manual mode **********/
void Hmi::displayManual(void)
{
    m_time = millis();
    if (m_manualWatering == 0) {
        m_manualWatering = Way::getFirst();
    }
    else {
        m_manualWatering = Way::getNext();
        if (m_manualWatering == 0) {
            m_manualWatering = Way::getFirst();
        }
    }
    display.displayManualWatering(m_manualWatering);
    m_state = DISPLAY_MANUAL;
}


/********** Display next watering **********/
void Hmi::displayNextWatering(void)
{
    time_t t;
	Watering *w;

    if (Watering::getNextWateringTime(&w, &t)) {
        display.displayNextWatering(w, t);
    } else {
        display.clearLine(LINE_1);
        display.clearLine(LINE_2);
        display.clearLine(LINE_3);
        display.clearLine(LINE_4);
    }
}


/********** Run function **********/
void Hmi::run(void)
{
    functionButton.loop();
    manualButton.loop();
    switch (m_state) {

        case IDLE:
            if (manualButton.isReleased()) {
                // DEBUG Serial.printf("# MANUAL\n");
                displayManual();
            }
            break;

        case DISPLAY_MANUAL:
            if (millis() - m_time > 8000) {
                displayNextWatering();
                m_state = IDLE;
                m_manualWatering = 0;
            }
            if (manualButton.isReleased()) {
                displayManual();
            }
            if (functionButton.isReleased()) {
                m_manualWatering->manualStart(Watering::manualDuration());
                displayNextWatering();
                m_state = IDLE;
                m_manualWatering = 0;
            }
            break;
    }
}


/********** isBusy function **********/
bool Hmi::isBusy(void)
{
    return m_state != IDLE;
}