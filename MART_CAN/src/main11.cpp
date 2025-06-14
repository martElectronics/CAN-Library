
#include <Arduino.h>
#include <MART_CAN.h>

const int voltimetroPin = 14;

int recieved[2];
int analogValue[2];

CAN_BUS CAN(HardwareType::Transciever, MCP_SPEED_500, 1);


void setup(){
   Serial.begin(115200);
   pinMode(voltimetroPin, INPUT);

   if(CAN.error == 1){
      Serial.println("Error Initializing EScP32Can...");
   }
   
}


void loop()
{
   analogValue[0] = analogRead(voltimetroPin);
   Serial.print("Analog Value: ");
   CAN.printArray(analogValue);
   CAN.setPacket(5, analogValue);

   CAN.getPacket(2, recieved);
   Serial.print(" Recieved: ");
   CAN.printArray(recieved);
   CAN.send();

   
}