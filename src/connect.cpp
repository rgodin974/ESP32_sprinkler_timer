#include "connect.h"

#define NTP_SERVER  "pool.ntp.org"
#define SYNC_INT 86400 // One day in sec -> 24 * 60 min * 60 sec

/********** Constructor **********/
ConfigConnect::ConfigConnect() {}
const char *ConfigConnect::_ssid;
const char *ConfigConnect::_password;


/********** WIFI events **********/
void ConfigConnect::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Connected to AP successfully!");
}

void ConfigConnect::WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.printf("WiFi connected to %s\n", _ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void ConfigConnect::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("Disconnected from WiFi access point");
    Serial.print("WiFi lost connection. ");
    Serial.printf("Trying to reconnect to : %s\n", _ssid);
    WiFi.reconnect();
}


/********** Display current system time **********/
void ConfigConnect::printTimeInfo()
{
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time");
      return;
    }
    Serial.println(&timeinfo, "SNTP synchronized : %A, %d %B %Y %H:%M:%S");
}


/********** Time synchronization callback **********/
void ConfigConnect::timeSyncCallback(struct timeval *tv)
{
	configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", NTP_SERVER);
    printTimeInfo();
}


/********** Init WIFI **********/
void ConfigConnect::initWiFi(const char *ssid, const char *password) {
    _ssid = ssid;
    _password = password;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    WiFi.begin(_ssid, _password);

    WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    while (WiFi.status() != WL_CONNECTED) {}
}


/********** Init SNTP **********/
void ConfigConnect::initSNTP() {
    sntp_set_time_sync_notification_cb(ConfigConnect::timeSyncCallback);
	sntp_set_sync_interval(SYNC_INT);
    Serial.printf("Sync. time set to %d secs\n", sntp_get_sync_interval());
	configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", NTP_SERVER);
}
