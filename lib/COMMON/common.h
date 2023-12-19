#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>

struct CanPacketRawData {
    unsigned long id;
    byte size;
    byte bytes[8];
    bool rrf;
    byte typeExtendedId;
};


#endif // COMMON_H