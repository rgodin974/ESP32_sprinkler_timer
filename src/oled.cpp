
#include <Arduino.h>
#include <WiFi.h>

#include "constants.h"
#include "oled.h"

Oled display;

/********** Constructor **********/
Oled::Oled() :
  Adafruit_SSD1327(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000)
{
}


/********** Initialization **********/
bool Oled::begin(void)
{
    if (!Adafruit_SSD1327::begin(SCREEN_ADDRESS)) {
        return false;
    }
    clearDisplay();
    setTextColor(SSD1327_WHITE);
    setTextSize(1);
    char title[] = "ESP32 loading...";
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(title, 0, 0, &x1, &y1, &w, &m_textHeight);
    setCursor((SCREEN_WIDTH - w) / 2, LINE_FLOW);
    print(title);
    display();
    return true;
}


/********** Display date and time (top full width) **********/
void Oled::displayTimeDate(void)
{
    char dateString[MAX_TEXT];
    char timeString[MAX_TEXT];
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    else {
        strftime(dateString, sizeof(dateString), "%a %d %b %y", &timeinfo);
        strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
    }
    fillRect(0, 0, SCREEN_WIDTH, m_textHeight, SSD1327_BLACK);
    setTextSize(1);
    setCursor(0, 0);
    print(dateString);
    
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(timeString, 0, 0, &x1, &y1, &w, &h);
    setCursor(SCREEN_WIDTH - w, 0);
    print(timeString);
    display();
}


/********** Display moisture (bottom left) **********/
void Oled::displayMoisture(int moisture)
{
    char buffer[MAX_TEXT];
    snprintf(buffer, MAX_TEXT, "RH %d%%", moisture);
    setTextSize(1);
    fillRect(0, SCREEN_HEIGHT - m_textHeight, 42, m_textHeight, SSD1327_BLACK); // 6x 7 chars -> RH 23%
    setCursor(0, SCREEN_HEIGHT - m_textHeight);
    print(buffer);
    display();
}


/********** Display flow (top middle) **********/
void Oled::displayFlow(float flow)
{
    uint16_t x;
    char buffer[MAX_TEXT];
    
    snprintf(buffer, MAX_TEXT, "%.1fL", flow);
    setTextSize(2);
    fillRect(0, LINE_FLOW , SCREEN_WIDTH, m_textHeight * 2, SSD1327_BLACK);
    textCenter(buffer, &x);
    setCursor(x, LINE_FLOW);
    print(buffer);
    display();
}


/********** Display messages (bottom middle) **********/
void Oled::displayMessage(const char *msg)
{
    int16_t x1, y1;
    uint16_t w, h;
    setTextSize(1);
    getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
    fillRect(0, LINE_MSG, SCREEN_WIDTH, m_textHeight, SSD1327_BLACK);
    setCursor((SCREEN_WIDTH - w) / 2, LINE_MSG);
    print(msg);
    display();
}


/********** Clear messages **********/
void Oled::clearMessage(void)
{
    fillRect(0, LINE_MSG, SCREEN_WIDTH, m_textHeight, SSD1327_BLACK);
}


/********** Display IP (bottom right) **********/
void Oled::displayIP(void)
{
    setTextSize(1);
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(WiFi.localIP().toString(), 0, 0, &x1, &y1, &w, &h);
    fillRect(86 , SCREEN_HEIGHT - m_textHeight, SCREEN_WIDTH, h, SSD1327_BLACK); // 128px - 42px from moisture = 86px
    setCursor(SCREEN_WIDTH - w, SCREEN_HEIGHT - m_textHeight);
    print(WiFi.localIP());
    display();
}


/********** Funtion to center text **********/
void Oled::textCenter(const char *s, uint16_t *x)
{
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
    *x = (SCREEN_WIDTH - w) / 2;
}


/********** Function to clear full line **********/
void Oled::clearLine(uint16_t y)
{
    fillRect(0, y, SCREEN_WIDTH, m_textHeight, SSD1327_BLACK);
}


/********** Display next watering (center screen) **********/
void Oled::displayNextWatering(Watering *w, time_t t)
{
    struct tm *pTime;
    char buffer[MAX_BUF];
    uint16_t x;

    setTextSize(1);
    clearLine(LINE_1);
    textCenter("NEXT WATERING", &x);
    setCursor(x, LINE_1);
    print("NEXT WATERING");

    clearLine(LINE_2);
    snprintf(buffer, MAX_TEXT, "%s - %s", w->getWay()->getZone()->getName(), w->getWay()->getShortName());
    textCenter(buffer, &x);
    setCursor(x, LINE_2);
    print(buffer);

    clearLine(LINE_3);
    pTime = localtime(&t);
    strftime(buffer, MAX_BUF, "%a %d %b %y %H:%M", pTime);
    textCenter(buffer, &x);
    setCursor(x, LINE_3);
    print(buffer);

    clearLine(LINE_4);
    snprintf(buffer, MAX_TEXT, "%d min", w->getDuration());
    textCenter(buffer, &x);
    setCursor(x, LINE_4);
    print(buffer);

    display();
}


/********** Display manual mode **********/
void Oled::displayManualWatering(Way *way)
{
    char buffer[MAX_BUF];
    uint16_t x;

    setTextSize(1);
    clearLine(LINE_1);
    textCenter("MANUAL WATERING", &x);
    setCursor(x, LINE_1);
    print("MANUAL WATERING");

    clearLine(LINE_2);
    snprintf(buffer, MAX_TEXT, "%s - %s", way->getZone()->getName(), way->getShortName());
    textCenter(buffer, &x);
    setCursor(x, LINE_2);
    print(buffer);

    clearLine(LINE_3);
    snprintf(buffer, MAX_TEXT, "%d min", Watering::manualDuration());
    textCenter(buffer, &x);
    setCursor(x, LINE_3);
    print(buffer);

    clearLine(LINE_4);

    display();
}