
#include <Arduino.h>
#include <MART_CAN.h>

CAN_BUS CAN(HardwareType::Transciever, MCP_SPEED_500, 1);

int analogValue[2];

int recieved[2];
void setup(){
   Serial.begin(115200);
   if(CAN.error == 1){
      Serial.println("Error Initializing EScP32Can...");
   }
}



void loop()
{
   CAN.getPacket(5, analogValue);
   if(analogValue[0] > 0){
      recieved[0] = 1;
   }
   CAN.printArray(analogValue);
   CAN.setPacket(2, recieved);
   CAN.send();
}