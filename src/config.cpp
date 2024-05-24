
#include "config.h"

Config *Config::m_config;
Config *Config::getConfig(void) {return m_config;}

/********** Constructor **********/
Config::Config(const char *fileName) :
  SPIFFSIniFile(fileName) {
  m_fileName = fileName;
  m_ssid[0] = '\0';
  m_password[0] = '\0';
  m_moistureSensor = 0;
  m_maxMoisture = 0;
  m_config = this;
}

/********** Get relays **********/
bool Config::getRelays(void) {
    char buffer[MAX_LINE];
    const char *p;
    char *s;
    int id;

    id = 0;
    // DEBUG Serial.printf("config::getRelays\n"); 
    if (getValue("relays", "relays", buffer, sizeof(buffer)) != true) {
        Serial.printf("%s:relays: GPIO not found (error %d)\n", m_fileName, getError());
    }
    p = strtok_r(buffer, ", ", &s);
    while (p != NULL) {
        // DEBUG Serial.printf("config::getRelays %s\n", p);
        if (Relay::create(id, p) != true) {
            Serial.printf("%s: error creating relay\n", p);
            return false;
        }
        p = strtok_r(NULL, ", ", &s);
        id++;
    }
    p = strtok_r(NULL, ", ", &s);
    return true;
}


/********** Get SSID **********/
char *Config::getSsid(void) {return m_ssid;}


/********** Get password **********/
char *Config::getPassword(void) {return m_password;}


/********** Moisture sensor **********/
uint8_t Config::getMoistureSensor(void) {return m_moistureSensor;}
uint8_t Config::getMaxMoisture(void) {return m_maxMoisture;}


/********** Flow sensor **********/
uint8_t Config::getFlowSensor(void) {return m_flowSensor;}
uint8_t Config::getMaxFlow(void) {return m_maxFlow;}


/********** Get zones **********/
bool Config::getZones(void) {
	char zones[MAX_LINE];
	const char *p;
	char *s;

	// DEBUG Serial.printf("config::getZones\n");
	if (getValue("zones", "zones", zones, sizeof(zones)) != true) {
		Serial.printf("%s:zones:zones not found (error %d)\n", m_fileName, getError());
		return false;
	}
	p = strtok_r(zones, ", ", &s);
	while (p != NULL) {
		// DEBUG Serial.printf("config::getZones %s\n", p);
		Zone::create(p);
		p = strtok_r(NULL, ", ", &s);
	}
	return true;
}


/********** Get ways **********/
bool Config::getWays(void)
{
    char buffer[MAX_LINE];
    const char *p;
    char *s;
    int id;

    Zone *z = Zone::getFirst();
    // DEBUG Serial.printf("config::getWays %p\n", z);
    while (z != NULL && strlen(z->getName()) != 0) {
        id = 0;
        // DEBUG Serial.printf("config::getWays %s\n", z->getName());
        if (getValue(z->getName(), "ways", buffer, sizeof(buffer)) != true) {
            Serial.printf("%s:ways: %s not found (error %d)\n", m_fileName, "ways", getError());
        } else {
            // DEBUG Serial.printf("%s:ways: %s\n", m_fileName, buffer);
        }
        p = strtok_r(buffer, ", ", &s);
        while (p != NULL) {
            // DEBUG Serial.printf("config::getWays %s %s\n", z->getName(), p);
            if (Way::create(z, p) != true) {
                Serial.printf("%s: error creating way\n", p);
                return false;
            }
            p = strtok_r(NULL, ", ", &s);
            id++;
        }
        p = strtok_r(NULL, ", ", &s);
        z = Zone::getNext();
    }
    return true;
}

/********** Read config **********/
bool Config::read(void) {
    char buffer[MAX_LINE];
    const char *p;
    char *s;

    if (!open()) {
        Serial.printf("%s: not found\n", m_fileName);
        return false;
    }

    // Get WIFI ssid
    if (getValue("WIFI", "access-point", buffer, sizeof(buffer)) != true) {
        Serial.printf("%s:WIFI:access-point not found (error %d)\n", m_fileName, getError());
        return false;
    }
    p = strtok_r(buffer, ":", &s);
    if (p == NULL) {
        Serial.printf("%s: bad format, missing ':'\n", buffer);
        return false;
    }
    strcpy(m_ssid, p);

    // Get WIFI password
    p = strtok_r(NULL, ":", &s);
    if (p == NULL) {
        Serial.printf("%s: missing password\n", buffer);
        return false;
    }
    strcpy(m_password, p);

    // Get relays
    if (getRelays() != true) {
        return false;
    }

	// Get zones
	if (getZones() != true) {
		return false;
	}

    // Get ways
    if (getWays() != true) {
        return false;
    }

    // Get manual duration
    int duration;
    if (getValue("manual", "duration", buffer, sizeof(buffer), duration) != true) {
        Serial.printf("%s:manual:duration not found (error %d)\n", m_fileName, getError());
        duration = 5;
    }
    Serial.printf("manual::duration %d min\n", duration);
    Watering::manualDuration(duration);

    // Get moisture sensor's values
    if (getValue("moisture", "sensor", buffer, sizeof(buffer)) == true) {
        m_moistureSensor = atoi(buffer);
        if (getValue("moisture", "max", buffer, sizeof(buffer)) == true) {
        m_maxMoisture = atoi(buffer);
        Serial.printf("moisture sensor::GPIO-%d, max value %d\n", m_moistureSensor, m_maxMoisture);
        }
    }

    // Get flow sensor's values
    if (getValue("flow", "sensor", buffer, sizeof(buffer)) == true) {
        m_flowSensor = atoi(buffer);
        if (getValue("flow", "max", buffer, sizeof(buffer)) == true) {
        m_maxFlow = atoi(buffer);
        Serial.printf("flow sensor::GPIO-%d, max value %d\n", m_flowSensor, m_maxFlow);
        }
    }

	// Get main valve
	// DEBUG Serial.printf("getting main valve\n");
	if (getValue("valve", "main", buffer, sizeof(buffer)) != true) {
		Serial.printf("%s:valve:main not found (error %d)\n", m_fileName, getError());
		return false;
	}
	if (Valve::create(buffer) != true) {
		Serial.printf("%s: error creating valve\n", buffer);
		return false;
	}

    return true;
}

/********** Print config **********/
void Config::print(void) {

	// Relays
	Serial.printf("relays: %d/%d\n", Relay::getCount(), MAX_WAY);
    Relay *relay = Relay::getFirst();
    while (relay != 0 && relay->isPresent()) {
        relay->print();
        relay = Relay::getNext();
    }

	// Main valve
  	// DEBUG : valeurs erronées... Valve::getMainValve()->print();

	// Zones
  	Serial.printf("zones: %d/%d\n", Zone::getCount(), MAX_ZONE);
	Zone *zone = Zone::getFirst();
	while (zone != 0) {
		zone->print();
		zone = Zone::getNext();
	}

    // Ways
    Serial.printf("ways: %d/%d\n", Way::getCount(), MAX_WAY);
    Way *way = Way::getFirst();
    while (way != 0) {
        way->print();
        way = Way::getNext();
    }
}