
//*******This sketch sends packets with id 100 and 200 every 5 and 2 seconds respectively

#include <Arduino.h>
#include "MART_CAN.h"


CAN_BUS can(5);
unsigned long canId = 100;
unsigned long canId2 = 200;
short iShort1[1] = {257};

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


