#include <Arduino.h>
#include <MART_CAN.h>


CAN_BUS CAN(5);
unsigned long canid = 100;
unsigned long canid1 = 101;
unsigned long canid2 = 102;
unsigned long canid3 = 103;
unsigned long canid4 = 104;
short iShort1[1] ;
short iShort2[2] ;
int iInt1[1] ;
int iInt2[2] ;
float iFloat1[1] ;
float iFloat2[2] ;

unsigned long removeableIDs[2]={101,103};
void setup()
{
   Serial.begin(9600);
   //Add to don't store permanently any packet in memory
   //CAN.DataIN.addRemovableIds()
   //Add to don't store permanently any packet whose id is in the removeableIDs array
   //CAN.DataIN.addRemovableIds(removeableIDs,2)
}

void loop()
{

   CAN.receive();
   CAN.getPacket(canid, iInt1, iShort2);
   CAN.getPacket(canid1, iInt2);
   CAN.getPacket(canid2, iFloat2);
   CAN.getPacket(canid3, iFloat1, iInt1);

   CAN.printArray(iInt1);
   // CAN.printArray(iShort2);
   // CAN.printArray(iFloat2);
   // CAN.printArray(iBool16);
}

    