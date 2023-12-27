
#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS can(5);

std::vector<uint16_t> testIds = {0x101, 0x102,0x109,0x110};
std::vector<uint16_t> testIds2 = {0x101, 0x102,0x109,0x110,0x54,0x4568,0x00,0x4};
void setup()
{
    Serial.begin(9600);
    can.setFilters(testIds);
    can.testFilters(testIds2);
}

void loop()
{
    
}