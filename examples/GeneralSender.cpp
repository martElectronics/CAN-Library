#include <Arduino.h>
#include <MART_CAN.h>

CAN_BUS CAN(5);
unsigned long canid = 100;
unsigned long canid1 = 101;
unsigned long canid2 = 102;
unsigned long canid3 = 103;
unsigned long canid4 = 104;

short iShort1[1] = {23};
short iShort2[2] = {23, 32};
int iInt1[1] = {5};
int iInt2[2] = {25, 30};
float iFloat1[1] = {3.14};
float iFloat2[2] = {1.23, 4.56};

void setup()
{
    Serial.begin(9600);
    CAN.setPacket(canid, iInt1, iShort2);
    CAN.setPacket(canid1, iInt2);
    CAN.setPacket(canid2, iFloat2);
    CAN.setPacket(canid3, iFloat1, iInt1);
}

void loop()
{
    CAN.send();
}
