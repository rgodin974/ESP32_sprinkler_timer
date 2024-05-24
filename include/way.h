
#ifndef _WAY_H_
#define _WAY_H_

#include <Ticker.h>
#include "zone.h"
#include "relay.h"

class Watering;

class Way
{
	public:
		Way();
		static bool create(Zone *z, const char *def);
		static int getCount();
		static Way *getFirst(void);
		static Way *getNext(void);
		static Way *getByName(const char *def);
		static bool isAnyManualWateringRunning(String &s);
		static void stopAllManualWatering(void);
		const char *getName(void); 	// Example ->Dripline.Flowers
		Relay *getRelay(void) {return m_relay;}
		void print(void);
		void open(void);
		void close(void);
		void manualStart(int duration);
		void manualStop(void);
		bool manualStarted(time_t *remain);
		Ticker *getTimer(void);

		// Ajout
		String getShortName(void) {return m_name;} // Example -> Flowers
		Zone *getZone(void) {return m_zone;} // Example -> Dripline

  	private:
		static Way m_way[MAX_WAY];
		static int m_searchIndex;
		Zone *m_zone;
		Relay *m_relay;
		Watering *watering[MAX_SCHEDULE];
		String m_name;  // Example -> Flowers
		String m_def;	// Example ->Dripline.Flowers
		time_t m_manualStarted;
		time_t m_manualDuration;
		Ticker m_timer;
};

#endif
