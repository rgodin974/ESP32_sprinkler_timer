
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <SPIFFS.h>
#include <FS.h>
#include <SPIFFSIniFile.h>

#include "relay.h"
#include "valve.h"
#include "zone.h"
#include "way.h"
#include "watering.h"

#define CONFIG_FILE "/config.ini"

class Config : public SPIFFSIniFile {
    public:
        Config(const char *fileName);
        bool read(void);
        void print(void);
        static Config *getConfig(void);
        char *getSsid(void);
        char *getPassword(void);
        uint8_t getMoistureSensor(void);
        uint8_t getMaxMoisture(void);
        uint8_t getFlowSensor(void);
        uint8_t getMaxFlow(void);

    private:
        static Config *m_config;
        const char *m_fileName;
        char m_ssid[MAX_LINE / 2];
        char m_password[MAX_LINE / 2];
        uint8_t m_moistureSensor;
        uint8_t m_maxMoisture;
        uint8_t m_flowSensor;
        uint8_t m_maxFlow;
        bool getRelays(void);
        bool getZones(void);
        bool getWays(void);
};

#endif