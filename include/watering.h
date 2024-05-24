
#ifndef _WATERING_H_
#define _WATERING_H_

#include "way.h"

#define DAY_DURATION    (24L*60L*60L)
#define DAY_WEEK		7

class Watering
{
    public:
        Watering();
        static int getCount();
        static Watering *getFirst(void);
        static Watering *getNext(void);
        static Watering *getWatering(int w);
        static Watering *getByName(const char *def, int index);
        static Watering *getFreeWatering(void);
        static Watering *getFreeWatering(const char *wayName);
        static bool create(int index, Way *way, const char *def);
        static bool run(time_t t);
        static const char *getNextWateringTime(Watering **w, time_t *t);    // ADD Watering **w parameter
        static bool getNextWayWateringTime(String &s); // ADD new function
        static bool isAnyWateringRunning(void);
        static bool isAnyAutoWateringRunning(String &s); // ADD new function
        static void stopAllAutoWatering(void);
        static void manualDuration(int duration);
        static int manualDuration();
        long getDuration(void);
        int getHour(void);
        int getMinute(void);
        int getIndex(void);
        Way *getWay(void);
        const char *getWayName(void);
        const char *getHourString(void);
        String getFrequency(void);     // ADD get watering frequency (Every days, odd, even, custom)
        int getClosestDay(Watering *w, int today);  // ADD get closest day for custom days
        time_t getStartTime(time_t now);
        time_t getStopTime(time_t now);
        char *getWateringDay(int i);    // ADD get custom watering days
        bool getEnable(void);           // ADD get enable state
        void toggleEnable();  // ADD set enable state
        bool always(void);
        bool even(void);                // ADD get even days state
        bool odd(void);                 // ADD get odd days state
        bool custom(void);              // ADD get custom days state
        void print(void);
        void autoStart(void);
        void autoStop(void);
        bool autoStarted(void);
        bool autoStarted(time_t *start, time_t *remain);    // ADD overload to get remain time for autostart
        void set(const char *wayName, int index);
        void set(int hour, int minute, long duration, const char *mode, char days[7][4]); // UPDATE (mode -> *, e, o, c)
        void set(const char *wayName, int index, int hour, int minute, long duration, bool always);

    private:
        static Watering m_watering[MAX_WATERING];
        static int m_searchIndex; // Search index 0->[MAX_WATERING]
        static int m_manualDuration;
        int m_index; // Index 0->3  for each way
        Way *m_way;
        int m_hour;
        int m_minute;
        long m_duration;
        bool m_enable;      // ADD enable and disable state
        bool m_always;
        bool m_even;        // ADD even days
        bool m_odd;         // ADD odd days
        bool m_custom;      // ADD custom days
        char m_wateringDays[DAY_WEEK][4];   // ADD array -> scheduled watering days (string days)
        time_t m_autoStarted;
        uint8_t m_moisture;
};

#endif
