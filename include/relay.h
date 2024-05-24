
#ifndef _RELAY_H_
#define _RELAY_H_

#include "constants.h"

enum relayState {ON, OFF};

class Relay {
    public:
        Relay();
        static bool create(int id, const char *def);
        static int getCount();
        static Relay *getByName(const char *name);
        static Relay *getFirst(void);
        static Relay *getNext(void);
        static Relay *getRelay(int n);
        bool isPresent(void);
        void print(const char*message = NULL);
        const char *getName(void);
        void setMode(uint8_t pin, uint8_t mode);
        void write(uint8_t pin, uint8_t value);
        void on(void);
        void off(void);
        relayState getState(void);


    private:
        static Relay m_relay[MAX_WAY];
        static int m_searchIndex;
        uint8_t m_index;
        uint8_t m_id;
        int8_t m_onPin;
        int8_t m_offPin;
        int8_t m_level;
        relayState m_state;
    };

#endif