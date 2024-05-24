#include <Arduino.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "connect.h"
#include "config.h"
#include "schedule.h"
#include "html.h"
#include "sensors.h"
#include "led.h"
#include "oled.h"
#include "hmi.h"
#include "manager.h"

Config config(CONFIG_FILE);
Schedule schedule(SCHEDULE_FILE);

AsyncWebServer server(80);
Hmi hmi;
bool sntp_sync;

void setup()
{
	Serial.begin(115200);

	// Oled
	if (!display.begin()) {
		Serial.println(F("SSD1327 allocation failed"));
		// DEBUG for (;;); // Don't proceed, loop forever
	}

	// Preferences manager -> Variable for seasonal adjustment
	prefs.begin("sprinkler");

	// Hmi
	hmi.begin();

	// Read config.ini file SPIFFS
	if (!SPIFFS.begin()) {
		Serial.println("SPIFFS.begin() failed");
	}

	// Read config file
	if (config.read() != true) {
		Serial.println("configuration failed");
		// while (1) sleep(1);
	}
	// DEBUG config.print();

	// Schedule file
	if (schedule.read() != true) {
		Serial.println("schedule configuration failed");
		// while (1) sleep(1);
	}
	// DEBUG schedule.print();

	// Reset all relays
	// DEBUG Serial.println("RESET all relays");
	Relay *relay = Relay::getFirst();
	while (relay != 0) {
		if (relay->isPresent()) {
			relay->off();
		}
			relay = Relay::getNext();
	}

	// Close main valve and wait for state -> VALVE_IS_CLOSED
	Valve::getMainValve()->close();
	while (Valve::getMainValve()->getState() != VALVE_IS_CLOSED) {}

	// Set WiFi parameters and SNTP server time
	ConfigConnect::initWiFi(config.getSsid(), config.getPassword());
	ConfigConnect::initSNTP();

	// Websocket
	initWebSocket();

  	// Main HTML page
	server.on("/", handleRoot);

	// Edit Schedules page
	server.on("/edit", handleEdit);
	server.on("/edit_submit", handleEditSubmit);
	server.on("/add", handleAdd);
	server.on("/remove", handleRemove);
	server.on("/weather", handleWeather);

  	// Files CSS
	server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/w3.css", "text/css");
	});

	server.on("/w3-theme.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/w3-theme.css", "text/css");
	});
  
	server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/style.css", "text/css");
	});

	server.on("/edit.css", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(SPIFFS, "/edit.css", "text/css");
	});

	// Files JS
	server.on("/fontawesome.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    	request->send(SPIFFS, "/fontawesome.min.js", "text/javascript");
	});

  	server.on("/scripts.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    	request->send(SPIFFS, "/scripts.js", "text/javascript");
	});

  	server.on("/edit.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    	request->send(SPIFFS, "/edit.js", "text/javascript");
	});

	// Start AsyncWebServer
	server.begin();

	// Init flow sensor
	flowInit();

	// Init LED
	ledInit();

	// Clear message
	display.clearMessage();
}

void loop()
{
	cleanUpClient();

	static time_t lastSec, lastFlow;
	time_t timestamp = time(NULL);
	float flow;

	if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
    	sntp_sync = true;
  	}
	
	if (sntp_sync) {
		if (timestamp > 10) {

			if (timestamp - lastFlow >= 1) {
				lastFlow = timestamp;
				flow = getFlow();
				display.displayFlow(flow); 			// Display to SSD1327
				
				if (flow > Config::getConfig()->getMaxFlow()) {
					// DEBUG Serial.printf("flow is too high\n");
					Watering::stopAllAutoWatering();
					Way::stopAllManualWatering();
					ledWrite(OVERFLOW_LED, ON);
				} else {
					ledWrite(OVERFLOW_LED, OFF);
				}
			}

			if (timestamp - lastSec >= 10) { // 60
				int moisture;
				getSoilMoisture(&moisture);
				if (!hmi.isBusy()) {
					hmi.displayNextWatering();		// Display to SSD1327
				}
				display.displayMoisture(moisture); 	// Display to SSD1327
				display.displayTimeDate();			// Display to SSD1327
				display.displayIP();				// Display to SSD1327
				lastSec = timestamp;
				Watering::run(timestamp);
			}
		}
		hmi.run();
	}
}
