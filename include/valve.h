
#ifndef _VALVE_H_
#define _VALVE_H_

#include <Arduino.h>
#include <Ticker.h>
#include "relay.h"

enum valveState {VALVE_UNKNOWN, VALVE_IS_CLOSED, VALVE_IS_OPENING, VALVE_IS_OPEN, VALVE_IS_CLOSING};
// UNECESSARY ? extern const char *valveStateName[];

class Valve
{
    public:
        Valve();
        static bool create(const char *def);
        static Valve *getMainValve(void);
        void open(void);
        void close(void);
        void isOpen(void);
        void isClosed(void);
        // void print(void); -> Valeurs erronées
        valveState getState(void) {
            return m_state;
        }

    private:
        static Valve m_mainValve;
        Relay *m_openRelay;
        Relay *m_closeRelay;
        valveState m_state;
};

#endif
