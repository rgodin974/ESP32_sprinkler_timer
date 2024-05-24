
#include "schedule.h"
#include "way.h"
#include "watering.h"


/********** Constructor **********/
Schedule::Schedule(const char *fileName) :
  SPIFFSIniFile(fileName)
{
  m_fileName = fileName;
}

/********** Read schedule file **********/
bool Schedule::read(void)
{
    if (!open()) {
        Serial.printf("%s: not found\n", m_fileName);
        return false;
    }
    Way *way = Way::getFirst();
    while (way != 0) {
        // DEBUG Serial.printf("schedule::read: %s\n", way->getName());
        for (int i = 0 ; i < MAX_SCHEDULE ; i++) {
            char name[MAX_DEF];
            char buffer[MAX_LINE];
            snprintf(name, MAX_DEF, "schedule%d", i + 1);
            if (getValue(way->getName(), name, buffer, sizeof(buffer)) != true) { // way->getName() -> Dripline.Flowers
                break;
            }
            // DEBUG Serial.printf("schedule::read: %s: %s\n", way->getName(), buffer);
            Watering::create(i, way, buffer);
        }
        way = Way::getNext();
    }
    return true;
}


/********** Write schedule configuration file **********/
bool Schedule::write(void)
{
    if (!open()) {
        Serial.printf("%s: not found\n", m_fileName);
        return false;
    }
    close();
    File file = SPIFFS.open(SCHEDULE_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("ERROR opening the file for writing");
        return false;
    }
    Way *way = Way::getFirst();
    while (way != 0 && way->getName() != 0) {
        // DEBUG Serial.printf("Schedule::write: %s\n", way->getName());
        file.printf("\n[%s]\n", way->getName());
        for (int i = 0 ; i < MAX_SCHEDULE ; i++) {
            Watering *w = Watering::getByName(way->getName(), i);
            if (w != 0) {
                // if (w->getDuration() != 0) {
                    if (w->always()) file.printf("schedule%d=%02d:%02d,%ld,*\n", i + 1, w->getHour(), w->getMinute(), w->getDuration());
                    if (w->even())   file.printf("schedule%d=%02d:%02d,%ld,e\n", i + 1, w->getHour(), w->getMinute(), w->getDuration());
                    if (w->odd())    file.printf("schedule%d=%02d:%02d,%ld,o\n", i + 1, w->getHour(), w->getMinute(), w->getDuration());
                    
                    if (w->custom()) {
                        char days[MAX_BUF] = "";
                        for (int n = 0; n < DAY_WEEK; n++) {
                            if(strlen(days) != 0 && strlen(w->getWateringDay(n)) != 0) {
                                strcat(days, ",");
                            }
                            strcat(days, w->getWateringDay(n));
                        }
                        file.printf("schedule%d=%02d:%02d,%ld,c,(%s)\n", i + 1, w->getHour(), w->getMinute(), w->getDuration(), days);
                    }
                // }
            }
        }
        way = Way::getNext();
    }
    file.close();
    return true;
}

/********** Print schedule data **********/
void Schedule::print(void)
{
    Serial.printf("watering: %d/%d\n", Watering::getCount(), MAX_WATERING);
    Watering *w = Watering::getFirst();
    while (w != 0) {
        if (w->getWay() != 0) {
            w->print();
        }
        w = Watering::getNext();
    }
}
