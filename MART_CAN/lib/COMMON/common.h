#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

// Uncomment the following line to enable debugging
#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.print(x)
#define DEBUG_PRINTLN(x)  Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

struct CanPacketRawData {
    unsigned long id;
    byte size;
    byte bytes[8];
    bool rrf;
    byte typeExtendedId;
    bool WaitForRRF;
};

#endif // COMMON_H