#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS can(5);
unsigned long canId = 100;
bool arrayBoolOUT[64] =
    {1, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0,0,0,0};

bool arrayBoolOUT16[16] =
    {1, 0, 1, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 1, 1, 1
    };

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


void printBoolArray(bool *a,int size);

void setup()
{


  Serial.begin(9600);

  can.setPacket(canId, iShort1,arrayBoolOUT16,iFloat1);
  const CanPacketRawData *packet = can.DataOUT.getPacketById(canId);
  
  if (packet != nullptr)
  {
    Serial.print("OUT: ");
    can.printArray(packet->bytes);
  }
  else
  {
    Serial.println("No matching packet found.");
  }
  // can.getPacket(canId, iShort1,iInt1,arrayBoolIN8_1,arrayBoolIN8_2);
 can.getPacket(canId, iShort1,arrayBoolIN16,iFloat1);
 
    can.printArray(iShort1);
  can.printArray(iFloat1);
printBoolArray(arrayBoolIN16,16);
    // printBoolArray(arrayBoolIN8_1,8);
    //   printBoolArray(arrayBoolIN8_2,8);

  // can.DataOUT.getPacketById(canId);

  can.DataIN=can.DataOUT;
}

void loop()
{

}

int myFunction(int x, int y)
{
  return x + y;
}

void printBoolArray(bool *a,int size)
{
   Serial.println();
  for(int i=0;i<size;i++)
  {
    if(i%8==0)  Serial.print(" ");  
    Serial.print(a[i]);
  }
  Serial.println();
}