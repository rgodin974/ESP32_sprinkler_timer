
#ifndef _ZONE_H_
#define _ZONE_H_

#include <Arduino.h>
#include "constants.h"

class Zone {
	public:
		Zone(void);
		static bool create(const char *def);
		static Zone m_zone[MAX_ZONE];
		static int getCount();
		static Zone *getFirst(void);
		static Zone *getNext(void);
		const char *getName(void);
		void print(void);

	private:
		static int m_searchIndex;
		String m_name;
};

#endif
