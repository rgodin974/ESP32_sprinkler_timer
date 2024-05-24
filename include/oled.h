
#ifndef _OLED_H_
#define _OLED_H_

#include <Adafruit_SSD1327.h>
#include "watering.h"

#define OLED_RESET      -1
#define SCREEN_ADDRESS  0x3C
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   128
#define MAX_TEXT        30

#define LINE_FLOW   20
#define LINE_1      50
#define LINE_2      67
#define LINE_3      79
#define LINE_4      91
#define LINE_MSG    108

class Oled : public Adafruit_SSD1327
{
  public:
    Oled();
    bool begin(void);
    void displayTimeDate(void);
    void displayMoisture(int moisture);
    void displayFlow(float flow);
    void displayMessage(const char *msg);
    void clearMessage(void);
    void displayIP(void);
    void displayNextWatering(Watering *w, time_t t);
    void displayManualWatering(Way *way);
    void textCenter(const char *s, uint16_t *x);
    void clearLine(uint16_t y);

  private:
    uint16_t m_textHeight;
};

extern Oled display;

#endif