
#ifndef _WIFI_H_
#define _WIFI_H_

#include <WiFi.h>
#include <esp_sntp.h>

class ConfigConnect {
    public :
        ConfigConnect();

        static void initWiFi(const char *ssid, const char *password);
        static void initSNTP();
        static void timeSyncCallback(struct timeval *tv);
        static void printTimeInfo();

        static void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
        static void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
        static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

    private :
        static const char *_ssid;
        static const char *_password;
};

#endif