#include <Arduino.h>
#include <MART_CAN.h>

CAN_BUS CAN(HardwareType::Transciever, MCP_SPEED_1000, 2);

uint32_t a;
uint32_t analogValue;
uint32_t recieved;

void setup(){
   Serial.begin(115200);
   if(CAN.error == 1){
      Serial.println("Error Initializing EScP32Can...");
   }
}

void loop()
{
   a = 1;
   CAN.receive();
   CAN.getPacket(5, &analogValue, 1);
   if(analogValue > 100){
      recieved = 1;
   }
   Serial.print("Analog Value: ");
   Serial.println(analogValue);
   Serial.print("recieved: ");
   CAN.setPacket(2, &recieved, 1);
   Serial.println(recieved);
   CAN.setPacket(3, &a, 1);
   CAN.send();

   recieved = 0;
}