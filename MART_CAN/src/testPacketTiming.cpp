#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS can(5);
unsigned long canId = 100;

unsigned long canId2 = 200;

bool arrayBoolIN[64];
bool arrayBoolIN16[16];
bool arrayBoolIN8_1[8];
bool arrayBoolIN8_2[8];

short iShort1[1] = {257};
short iShort2[2] = {23, 32};
int iInt1[1] = {258};
int iInt2[2] = {25, 30};
float iFloat1[1] = {3.14};
float iFloat2[2] = {1.23, 4.56};

void printBoolArray(bool *a, int size);

void setup()
{

  Serial.begin(9600);
  can.setPacket(canId, iShort1);
  can.setPacket(canId2, iShort1);
  can.setPacketTimer(canId,5000);
  can.setPacketTimer(canId2,2000);
  
}

void loop()
{
    can.send();
}


