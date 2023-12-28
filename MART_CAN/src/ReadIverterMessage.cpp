//*******This sketch is prepared to read the inverter message with id 20;
#include <Arduino.h>
#include <MART_CAN.h>

// put function declarations here:

CAN_BUS CAN(5);
unsigned long canid = 20;

short DutyCycle[1] ;
short InputVoltage[1] ;
int ERPM[1] ;

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
   //Lee el mensaje con id 0x20 del inversor
   CAN.receive();
   CAN.getPacket(canid, DutyCycle, InputVoltage); 
   Serial.println("***INVERTER DATA***");
   Serial.print("Electrical RPM : ");
   CAN.printArray(ERPM); 
   Serial.print("Duty Cycle");
   CAN.printArray(DutyCycle);
   Serial.print("Input Voltage");
   CAN.printArray(InputVoltage);
}

    