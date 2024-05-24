
#include <Arduino.h>
#include "relay.h"

int Relay::m_searchIndex;
Relay Relay::m_relay[MAX_WAY];

/********** Constructor **********/
Relay::Relay() : m_index(0),
				 m_id(0),
				 m_onPin(-1), m_offPin(-1),
				 m_level(HIGH)
{
}


/********** Create a relay **********/
bool Relay::create(int id, const char *def)
{
	Relay *relay;
	char tmp[MAX_DEF];
	const char *p, *prefix;
	char *s;

	relay = Relay::getRelay(m_searchIndex);
	if (relay == 0)
	{
		Serial.printf("relay %d: too much items !!!\n", m_searchIndex);
		return false;
	}
	relay->m_index = m_searchIndex;
	relay->m_id = id;
	strncpy(tmp, def, MAX_DEF);
	prefix = strtok_r(tmp, "(", &s);
	if (prefix == NULL)
	{
		return false;
	}
	if (!strcmp(prefix, "GPIO-H"))
	{
		relay->m_level = HIGH;
	}
	else if (!strcmp(prefix, "GPIO-L"))
	{
		relay->m_level = LOW;
	}
	else
	{
		Serial.printf("%s: bad value\n", def);
		return false;
	}
	p = strtok_r(NULL, ")", &s);
	if (p == NULL)
	{
		Serial.printf("%s: bad format, missing parenthesis\n", def);
		return false;
	}
	relay->m_onPin = atoi(p);
	relay->setMode(relay->m_onPin, OUTPUT);
	relay->write(relay->m_onPin, relay->m_level == HIGH ? LOW : HIGH);
	m_searchIndex++;
	return true;
}

/********** Set relay's GPIO mode **********/
void Relay::setMode(uint8_t pin, uint8_t mode)
{
	// DEBUG Serial.printf("%s: pin%d=OUTPUT\n", getName(), pin);
	pinMode(pin, mode);
}

/********** Write HIGH or LOW to the module's GPIO **********/
void Relay::write(uint8_t pin, uint8_t value)
{
	digitalWrite(pin, value);
}


/********** Return number of relays **********/
int Relay::getCount()
{
	int n;

	for (n = 0; n < MAX_WAY && m_relay[n].isPresent() == true; n++);
	return n;
}


/********** Return relay name **********/
const char *Relay::getName(void)
{
	static char name[MAX_DEF];
	snprintf(name, MAX_DEF, "GPIO-%d", m_onPin);
	return name;
}

/********** Return a relay giving its name **********/
Relay *Relay::getByName(const char *def)
{
	Relay *relay = Relay::getFirst();
	while (relay != 0) {
		if (!strcmp(relay->getName(), def)) {
			// DEBUG Serial.printf("relay::getByName found %s\n", def);
			return relay;
		}
		relay = Relay::getNext();
	}
	return 0;
}


/********** Return first relay **********/
Relay *Relay::getFirst(void)
{
	m_searchIndex = 0;
	return &m_relay[m_searchIndex];
}


/********** Return next relay **********/
Relay *Relay::getNext(void)
{
	m_searchIndex++;
	if (m_searchIndex < MAX_WAY)
	{
		return &m_relay[m_searchIndex];
	}
	return 0;
}


/********** Return specific relay **********/
Relay *Relay::getRelay(int n)
{
	if (n < MAX_WAY)
	{
		return &m_relay[n];
	}
	return 0;
}


/********** Get relay state **********/
relayState Relay::getState(void) {
    return m_state;
}


/********** Relay's presence **********/
bool Relay::isPresent(void)
{
  return (m_onPin != -1 || m_offPin != -1);
}


/********** Print relay **********/
void Relay::print(const char *message)
{
	if (message)
	{
		Serial.printf(message);
	}
	if (!isPresent()) {
	  Serial.printf("Not present\n");
	  return;
	} else {
	  Serial.printf("id: %d index: %d: %s, NORMAL, GPIO, %s(%d)\n", m_id, m_index, getName(), m_level == HIGH ? "HIGH-LEVEL" : "LOW-LEVEL", m_onPin);
	}
}


/********** Turn ON relay **********/
void Relay::on(void)
{
	if (!isPresent()) {
		return;
	}
	// DEBUG Serial.printf("%s: pin%d=%d\n", getName(), m_onPin, m_level);
	digitalWrite(m_onPin, m_level);
	m_state = ON;
}


/********** Turn OFF relay **********/
void Relay::off(void)
{
	if (!isPresent()) {
		return;
	}
	// DEBUG Serial.printf("%s: pin%d=%d\n", getName(), m_onPin, !m_level);
	digitalWrite(m_onPin, !m_level);
	m_state = OFF;
}