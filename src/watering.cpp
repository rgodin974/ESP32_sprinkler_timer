
#include <Arduino.h>
#include "watering.h"
#include "config.h"
#include "schedule.h"
#include "valve.h"
#include "sensors.h"
#include "manager.h"
// #include "oled.h"

int Watering::m_searchIndex;
int Watering::m_manualDuration;
Watering Watering::m_watering[MAX_WATERING];


/********** Constructor **********/
Watering::Watering() :
	m_way(0),
	m_hour(0), m_minute(0),
	m_duration(0),
	m_enable(true),
	m_always(false), m_even(false), m_odd(false), m_custom(false),
	m_wateringDays{"", "", "", "", "", "", ""},
	m_autoStarted(0),
	m_moisture(0)
{
}


/********** Create watering for a given way **********/
// Example for schedule1 def -> 07:00,15,*
bool Watering::create(int index, Way *way, const char *def)
{
	char tmp[MAX_DEF];
	Watering *w;
	const char *p;
	char *s;

	// DEBUG Serial.printf("watering::create %s %s\n", way->getName(), def);
	strncpy(tmp, def, MAX_DEF);
	w = getByName(way->getName(), index); // way->getName() -> Dripline.Flowers
	if (w) {
		w->m_index = index;
		w->m_way = way;

		// Get hours
		p = strtok_r(tmp, ":", &s);
		if (p == NULL) {
			Serial.printf("%s: bad format, missing ':'\n", def);
			return false;
		}
		w->m_hour = atoi(p);

		// Get minutes
		p = strtok_r(NULL, ",", &s);
		if (p == NULL) {
			Serial.printf("%s: bad format, missing ','\n", def);
			return false;
		}
		w->m_minute = atoi(p);

		// Get duration
		p = strtok_r(NULL, ",", &s);
		if (p == NULL) {
			Serial.printf("%s: bad format, missing ','\n", def);
			return false;
		}
		w->m_duration = atol(p);

		// Frequency watering
		p = strtok_r(NULL, ",", &s);
		w->m_always = false;
		w->m_even 	= false;
		w->m_odd	= false;
		w->m_custom = false;

		if (p != NULL && *p == '*') {
			w->m_always = true;	// Every days watering
		}
		if (p != NULL && *p == 'e') {
			w->m_even = true;	// Even days watering (2,4,...)
		}
		if (p != NULL && *p == 'o') {
			w->m_odd = true;	// Odd days watering (1,3,...)
		}
		if (p != NULL && *p == 'c') {
			w->m_custom = true;	// Custom days watering
		}

		// Watering days for custom days
		p = strtok_r(NULL, "(, )", &s);

		int i = 0;
		while (p != NULL) {
			strcpy(w->m_wateringDays[i], p);
			p = strtok_r(NULL, " ,)", &s);
			i++;
		}
	}
	return true;
}

/********** Return number of waterings **********/
int Watering::getCount()
{
	int n = 0;
	int i;

	for (i = 0 ; i < MAX_WATERING ; i++) {
		if (m_watering[n].m_way != 0) {
			n++;
		}
	}
	return n;
}

/********** Return first watering **********/
Watering *Watering::getFirst(void)
{
	m_searchIndex = 0;
	return &m_watering[m_searchIndex];
}


/********** Return next watering **********/
Watering *Watering::getNext(void)
{
	m_searchIndex++;
	if (m_searchIndex < MAX_WATERING && m_watering[m_searchIndex].m_way != 0) {
		return &m_watering[m_searchIndex];
	}
	return 0;
}


/********** Return way of watering **********/
Way *Watering::getWay(void) {return m_way;};


/********** Return way name of watering **********/
const char *Watering::getWayName(void) {return m_way->getName();}; // way->getName() -> Dripline.Flowers 


/********** Return watering giving its name **********/
Watering *Watering::getByName(const char *def, int index)
{
	// DEBUG Serial.printf("watering::getByName %s %d\n", def, index);
	for (int i = 0 ; i < MAX_WATERING ; i++) {
		Watering *w = &m_watering[i];
		if (w->m_way == 0) {
			return 0;
		}
		if (!strcmp(w->getWayName(), def) && w->m_index == index) {
			// DEBUG Serial.printf("watering::getByName found %s %d at %d\n", w->getWayName(), w->m_index, i);
			return w;
		}
	}
	return 0;
}


/********** Return first free watering **********/
Watering *Watering::getFreeWatering(void)
{
    // DEBUG Serial.printf("watering::getFreeWatering\n");
    for (int i = 0 ; i < MAX_WATERING ; i++) {
        Watering *w = &m_watering[i];
        if (w->m_way == 0) {
            // DEBUG Serial.printf("watering::getFreeWatering found %d\n", i);
            return w;
        }
    }
    return 0;
}


/********** Return a free watering for a way **********/
Watering *Watering::getFreeWatering(const char *wayName)
{
  // DEBUG Serial.printf("watering::getFreeWatering %s\n", wayName);
	for (int i = 0 ; i < MAX_WATERING ; i++) {
		Watering *w = &m_watering[i];
		if (w->m_way == 0) {
			return 0;
		}
		if (!strcmp(w->getWayName(), wayName)) {
			if (w->m_duration == 0) {
				// DEBUG Serial.printf("watering::getFreeWatering found %d\n", i);
				return w;
			}
		}
	}
	return 0;
}


/********** Run the watering **********/
bool Watering::run(time_t t)
{
	struct tm *pTime;
	pTime = localtime(&t);
	int toDay = pTime->tm_mday;   	// Month day 1 -> 31
	// int weekDay	= pTime->tm_wday;	// Week day  0 -> 6 (Sunday = 0)
	char toDayName[MAX_BUF];
	strftime(toDayName, MAX_BUF, "%a", pTime );
	//DEBUG Serial.printf("day:%s\n", dayName);

	for (int i = 0 ; i < MAX_WATERING ; i++) {
		Watering *w = &m_watering[i];
		if (w->getDuration() != 0) {
			time_t stopTime = w->getStopTime(t);
			int adjustment = prefs.getInt("season_adj", 100);		 					// Updated for seasonal adjustement function
			time_t startTime = stopTime - (w->getDuration() * 60 * adjustment) / 100;	// Updated for seasonal adjustement function
			pTime = localtime(&stopTime);
			bool isWateringDay = false;

			if (t >= startTime && t <= stopTime) {
				// Check if today is a watering custom day
				for (int n = 0; n < DAY_WEEK; n++) {
					if (strcmp(toDayName, w->m_wateringDays[n]) == 0) {
						isWateringDay = true;
						break;
					}
				}
				
				// Start waterings
				if (w->m_always || (w->m_even && toDay %2 == 0) || (w->m_odd && toDay %2 != 0) || (w->m_custom && isWateringDay)) {
					if (w->getEnable()) { // Run only if w->getEnable() ==  true otherwise don't start
						if (!w->autoStarted()) w->autoStart();
					}
				}

			} else {
				if (w->autoStarted()) w->autoStop();
			}
		}
	}
	if (isAnyWateringRunning() == false) {
		Valve::getMainValve()->close();
	}
	else {
		Valve::getMainValve()->open();
	}
	return true;
}


/********** Get next watering time **********/
const char *Watering::getNextWateringTime(Watering **watering, time_t *t)
{
	struct tm *pTime;
	time_t timestamp = time(NULL);

	pTime = localtime(&timestamp);
	int toDay = pTime->tm_wday;	   // Today week day 0->6 (sunday = 0)

	time_t nextWateringTime = 0;
	const char *nextWateringWay = 0;
	Watering *w = 0;

	for (int i = 0 ; i < MAX_WATERING ; i++) {
		w = &m_watering[i];
		if (w->getDuration() != 0 && w->getEnable()) {
			time_t startTime = w->getStartTime(timestamp);
			pTime = localtime(&startTime);
			int startDay = pTime->tm_mday; 		// Start day 1->31
			int startWeekDay = pTime->tm_wday;  // Start week day 0->6

			if (w->m_custom) {
				// Get closest day
				int closestDay = w->getClosestDay(w, toDay);

				// Day offset
				if (startWeekDay > toDay) {
					startTime += DAY_DURATION * (closestDay - startWeekDay);
					if (closestDay == toDay) {
						if (startTime < timestamp) {
							startTime += DAY_DURATION * 7;
						} else {
							startTime += DAY_DURATION ;
						}
					}
				}
				if (startWeekDay < toDay) {
					startTime += DAY_DURATION * (closestDay %7 - startWeekDay);
					if (closestDay == toDay) {
						if (startTime > timestamp) {
							startTime += DAY_DURATION;
						} else {
							startTime += DAY_DURATION * 7;
						}
					}
				}
				if (startWeekDay == toDay) {
					startTime += DAY_DURATION * (closestDay - startWeekDay);
				}
			}


			if (w->m_even && startDay %2 != 0) {
				startTime = startTime + DAY_DURATION;
			}

			if (w->m_odd && startDay %2 == 0) {
				startTime = startTime + DAY_DURATION;
			}

			if (nextWateringTime == 0) {
				nextWateringTime = startTime;
				nextWateringWay = w->getWayName();
				*watering = &m_watering[i];
			}

			if (nextWateringTime > startTime) {
				nextWateringTime = startTime;
				nextWateringWay = w->getWayName();
				*watering = &m_watering[i];
			}
		}
	}
	*t = nextWateringTime;
	// DEBUG Serial.printf("Watering::getNextWateringTime found %s %ld\n", nextWateringWay == 0 ? "none" : nextWateringWay, *t);
	return nextWateringWay;
}


/********** Get next watering time **********/
bool Watering::getNextWayWateringTime(String &s)
{
	s = "";
	struct tm *pTime;
	time_t timestamp = time(NULL);
	time_t nextWateringTime[MAX_WAY] = {0};

	pTime = localtime(&timestamp);
	int toDay = pTime->tm_wday;	   // Today week day 0->6 (sunday = 0)

	const char *nextWateringWay[MAX_WAY] = {0};
	int n = 0;
	char buffer[MAX_BUF];
	bool status = false;
	Watering *w = 0;

	// Get next watering time for each way into an array
	Way *way = Way::getFirst();
	while (way != 0) {
		for (int i = 0; i < MAX_SCHEDULE; i++) {
			w = Watering::getByName(way->getName(), i);
			if (w->getDuration() != 0 && w->getEnable()) {
				time_t startTime = w->getStartTime(timestamp);
				pTime = localtime(&startTime);
				int startDay = pTime->tm_mday; 		// Start day 1->31
				int startWeekDay = pTime->tm_wday;  // Start week day 0->6				

				if (w->m_custom) {
					// Get closest day
					int closestDay = w->getClosestDay(w, toDay);

					// Serial.printf("way[%s] today: %d\n", w->getWayName(), toDay);
					// Serial.printf("way[%s] start: %d\n", w->getWayName(), startWeekDay);
					// Serial.printf("way[%s] closest: %d\n", w->getWayName(), closestDay);

					// Day offset
					if (startWeekDay > toDay) {
						Serial.printf("1\n");
						startTime += DAY_DURATION * (closestDay - startWeekDay);
						if (closestDay == toDay) {
							Serial.printf("1bis\n");
							if (startTime < timestamp) {
								Serial.printf("1a\n");
								startTime += DAY_DURATION * 7;
							} else {
								Serial.printf("1b\n");
								startTime += DAY_DURATION ;
							}
						}
					}
					if (startWeekDay < toDay) {
						Serial.printf("2\n");
						startTime += DAY_DURATION * (closestDay %7 - startWeekDay);
						if (closestDay == toDay) {
							Serial.printf("2bis\n");
							if (startTime > timestamp) {
								Serial.printf("2a\n");
								startTime += DAY_DURATION;
							} else {
								Serial.printf("2b\n");
								startTime += DAY_DURATION * 7;
							}
						}
					}
					if (startWeekDay == toDay) {
						Serial.printf("3\n");
						startTime += DAY_DURATION * (closestDay - startWeekDay);
					}
				}

				if (w->m_even && startDay %2 != 0) {
					startTime += DAY_DURATION;
				}

				if (w->m_odd && startDay %2 == 0) {
					startTime += DAY_DURATION;
				}				

				if (nextWateringTime[n] == 0) {
					nextWateringTime[n] = startTime;
					nextWateringWay[n] 	= w->getWayName();
				}

				if (nextWateringTime[n] > startTime) {
					nextWateringTime[n] = startTime;
					nextWateringWay[n]	= w->getWayName();
				}
			} else {
				nextWateringWay[n] = w->getWayName();
			}
		}
		n++;
		way = Way::getNext();
	}

	// Concate array -> Lawn.Studio=Sun 05 May 24 08:00,Dripline.Flowers=Sat 04 May 24 15:00
	for (int i = 0; i <  MAX_WAY; i++) {
		if (nextWateringWay[i] !=0) {
			if (s.length() > 0) {
				s += ",";
			}
			s += nextWateringWay[i];
			s += "=";
			if (nextWateringTime[i] != 0) {
				pTime = localtime(&nextWateringTime[i]);
				strftime(buffer, MAX_BUF, "%a %d %b %y %H:%M", pTime);
				s+= "Next run scheduled on ";
				s += buffer;
			} else  {
				s+= "No irrigation(s) scheduled";
			}
			status = true;
		}
	}
	return status;
}


/********** Get closest day for custom days **********/
int Watering::getClosestDay(Watering *w, int today) {
	int days[7] = {-1, -1, -1, -1, -1, -1, -1};

	// Convert string array to int array
	for (int n = 0; n < DAY_WEEK; n++) {
		if (strcmp(w->m_wateringDays[n], "Sun") == 0 && strlen(w->m_wateringDays[n]) != 0) days[n] = 0;
		if (strcmp(w->m_wateringDays[n], "Mon") == 0 && strlen(w->m_wateringDays[n]) != 0) days[n] = 1;
		if (strcmp(w->m_wateringDays[n], "Tue") == 0 && strlen(w->m_wateringDays[n]) != 0) days[n] = 2;
		if (strcmp(w->m_wateringDays[n], "Wed") == 0 && strlen(w->m_wateringDays[n]) != 0) days[n] = 3;
		if (strcmp(w->m_wateringDays[n], "Thu") == 0 && strlen(w->m_wateringDays[n]) != 0) days[n] = 4;
		if (strcmp(w->m_wateringDays[n], "Fri") == 0 && strlen(w->m_wateringDays[n]) != 0) days[n] = 5;
		if (strcmp(w->m_wateringDays[n], "Sat") == 0 && strlen(w->m_wateringDays[n]) != 0) days[n] = 6;
	}

	// Find the closest day from today
	int minDifference = INT_MAX;
	int closestDay;
	for (int n = 0; n < DAY_WEEK; n++) {
		if (days[n] != -1) {
			if (days[n] < today) days[n] += 7;
			int difference = abs(today - days[n]);
			if (difference < minDifference) {
				minDifference = difference;
				closestDay = days[n];
			}
		}
	}

	return closestDay;
}


/********** Check if any watering is running **********/
bool Watering::isAnyWateringRunning(void)
{
	for (int i = 0 ; i < MAX_WATERING ; i++) {
		Watering *w = &m_watering[i];
		if (w->getDuration() != 0) {
			if (w->autoStarted() || w->getWay()->manualStarted(NULL)) {
				return true;
			}
		}
	}
	return false;
}


/********** Check if any auto watering is running and return remaining time**********/
bool Watering::isAnyAutoWateringRunning(String &s)
{
	s = "";
    char t[6];
    bool status = false;
    for (int i = 0 ; i < MAX_WATERING ; i++) {
        Watering *w = &m_watering[i];
		if (w->getDuration() != 0) {
			time_t remain;
			time_t start = w->getStartTime(time(NULL));
			if (w->autoStarted(&start ,&remain)) {
				// DEBUG Serial.printf("watering::isAnyAutoWateringRunning found %s\n", w->getWayName());
				if (s.length() > 0) {
					s += ",";
				}
				s += w->getWayName();
				s += "=";
				if (remain < 1) {
					remain = 0;
				}
				snprintf(t, 6, "%02ld:%02ld", remain / 60, remain % 60);
				s += t;
				status = true;
			}
		}
    }
    return status;
}


/********** Stop all auto waterings **********/
void Watering::stopAllAutoWatering(void)
{
	for (int i = 0 ; i < MAX_WATERING ; i++) {
		Watering *w = &m_watering[i];
		if (w->getDuration() != 0) {
			if (w->autoStarted()) {
				w->autoStop();
			}
		}
	}
}


/********** Get hour -> 10:15 **********/
const char *Watering::getHourString(void)
{
  struct tm *pTime;
  time_t t = time(NULL);
  static char buffer[MAX_BUF];
  time_t startTime = getStartTime(t);
  pTime = localtime(&startTime);
  strftime(buffer, MAX_BUF, "%H:%M", pTime);
  return buffer;
}


/********** Get frequency (always, odd, even, custom) **********/
String Watering::getFrequency(void)
{
	String s = "";
	if (m_always) return "Every days";
	if (m_even)	  return "Even days";
	if (m_odd)	  return "Odd days";

	if (m_custom) {
		for (int i = 0; i < DAY_WEEK; i++) {
			if (strlen(m_wateringDays[i]) != 0) {
				if (s.length() > 0) {
					s += ", ";
				}
				s += m_wateringDays[i];
			}
		}
		return s;
	}
	return "Error";
}


/********** Return watering's start time **********/
time_t Watering::getStartTime(time_t now)
{
  struct tm *pTime;

  pTime = localtime(&now);
  pTime->tm_hour = m_hour;
  pTime->tm_min = m_minute;
  pTime->tm_sec = 0;
  time_t at = mktime(pTime);
  if (at < now) {
    at += DAY_DURATION;
  }
  return at;
}

/********** Return watering's stop time **********/
time_t Watering::getStopTime(time_t now)
{
	struct tm *pTime;

	pTime = localtime(&now);
	int adjustment = prefs.getInt("season_adj", 100);	// Updated for seasonal adjustement function
	int ttime = (m_duration * 60 * adjustment) / 100;	// Convert m_duration from mins to secs
	int hour = ttime / 3600; 		// get m_duration -> hour
	int mins = (ttime / 60) % 60 ;	// get m_duration -> mins
	int secs = ttime % 60;			// get m_duration -> secs

	pTime->tm_hour = m_hour + hour;  	// Updated for seasonal adjustement function
	pTime->tm_min = m_minute + mins; 	// Updated for seasonal adjustement function
	pTime->tm_sec = secs;				// Update for seasonal adjustment function

	// pTime->tm_hour = m_hour + (m_duration / 60);
	// pTime->tm_min = m_minute + (m_duration % 60);
	// pTime->tm_sec = 0;
	time_t at = mktime(pTime);
	if (at < now) {
		at += DAY_DURATION;
	}
	return at;
}


/********** Print watering **********/
void Watering::print(void)
{
	Serial.printf("index:%d searchIndex:%d %s %02d:%02d %ld minutes\n",m_index , m_searchIndex, m_way->getName(), m_hour, m_minute, m_duration);
}


/********** Auto START **********/
void Watering::autoStart(void)
{
  int moisture;

  Serial.printf("Watering::autoStart %s: %ld\n", getWayName(), m_duration);
  m_autoStarted = time(NULL);
  if (m_moisture == 0) {
    m_moisture = getSoilMoisture(&moisture);
    if (m_moisture == HUMIDITY_DRY) {
      Serial.printf("Watering::autoStart: moisture %x (DRY)\n", moisture);
      m_way->open();
    }
    else {
      Serial.printf("Watering::autoStart: moisture %x (WET)\n", moisture);
    }
  }
}


/********** Auto STOP **********/
void Watering::autoStop(void)
{
  // DEBUG Serial.printf("Watering::autoStop %s\n", getWayName());
  m_autoStarted = 0;
  m_moisture = 0;
  if (!m_way->manualStarted(NULL)) {
    // DEBUG Serial.printf("close relay %s\n", m_way->getRelay()->getName());
    m_way->close();
  }
}


/********** Auto STARTED **********/
bool Watering::autoStarted(void)
{
	if (m_autoStarted != 0) {
		// DEBUG Serial.printf("watering::autoStarted: %s\n", getWayName());
		return true;
	}
	return false;
}

bool Watering::autoStarted(time_t *start, time_t *remain)
{
	if (m_autoStarted != 0) {
		// DEBUG Serial.printf("watering::autoStarted: %s\n", getWayName());
		if (remain) {
			time_t elapsed = time(NULL) - (*start - DAY_DURATION);
			int adjustment = prefs.getInt("season_adj", 100); 			// Updated for seasonal adjustement function
			*remain = ((m_duration * 60 * adjustment) / 100) - elapsed;	// Updated for seasonal adjustement function
		}
		return true;
	}
	return false;
}

/********** Set watering by way name and index **********/
void Watering::set(const char *wayName, int index)
{
  // DEBUG Serial.printf("Watering::set %s[%d]\n", wayName, index);
  Way *way = Way::getByName(wayName);
  m_way = way;
  m_index = index;
}


/********** Set watering : hour, minute, duration, frequency, custom days **********/
void Watering::set(int hour, int minute, long duration, const char *mode, char days[7][4])
{
	// DEBUG Serial.printf("Watering::set %s[%d] %d:%d %ld %s\n", getWayName(), m_index, hour, minute, duration, mode);
	m_hour = hour;
	m_minute = minute;
	m_duration = duration;

	m_always = !strcmp(mode, "*") ? true : false;
	m_even   = !strcmp(mode, "e") ? true : false;
	m_odd    = !strcmp(mode, "o") ? true : false;
	m_custom = !strcmp(mode, "c") ? true : false;
		
	if (!strcmp(mode, "c")) {
		for (int i = 0; i < DAY_WEEK; i++) {
			strcpy(m_wateringDays[i], days[i]);
		}
	} else {
		for (int i = 0; i < DAY_WEEK; i++) {
			strcpy(m_wateringDays[i], "");
		}
	}

  	schedule.write();
}


/********** Set watering : way, index, hour, minute, duration, mode **********/
void Watering::set(const char *wayName, int index, int hour, int minute, long duration, bool always)
{
	// DEBUG Serial.printf("Watering::set %s[%d] %d %d %ld %d\n", wayName, index, hour, minute, duration, always);
	Way *way = Way::getByName(wayName);
	m_way = way;
	m_index = index;
	m_hour = hour;
	m_minute = minute;
	m_duration = duration;
	m_always = always;
	m_even = false;
	m_odd = false;
	m_custom = false;
	for (int i = 0; i < DAY_WEEK; i++) {
		strcpy(m_wateringDays[i], "");
	}
	schedule.write();
}


/********** Set / Get manual duration **********/
void Watering::manualDuration(int duration) {m_manualDuration = duration;}
int Watering::manualDuration() {return m_manualDuration;}
Watering *Watering::getWatering(int w) {return &m_watering[w];}
long Watering::getDuration(void) {return m_duration;};
int Watering::getIndex(void) {return m_index;};
bool Watering::always(void) {return m_always;};
bool Watering::even(void) {return m_even;};
bool Watering::odd(void) {return m_odd;};
bool Watering::custom(void) {return m_custom;};
char *Watering::getWateringDay(int i){return m_wateringDays[i];};
int Watering::getHour(void) {return m_hour;};
int Watering::getMinute(void) {return m_minute;};
bool Watering::getEnable(void) {return m_enable;};
void Watering::toggleEnable() {m_enable = !m_enable;};