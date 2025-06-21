
#include <Arduino.h>
#include <MART_CAN.h>

const int voltimetroPin = 14;

uint32_t recieved;
uint32_t analogValue;

CAN_BUS CAN(HardwareType::Transciever, MCP_SPEED_1000, 1);


void setup(){
   Serial.begin(115200);
   pinMode(voltimetroPin, INPUT);

   if(CAN.error == 1){
      Serial.println("Error Initializing EScP32Can...");
   }
   
}


void loop()
{
   analogValue = analogRead(voltimetroPin);
   Serial.print("Analog Value: ");
   Serial.println(analogValue);
   CAN.setPacket(5, &analogValue,1);
   CAN.receive();
   CAN.printReceivedIds();

   CAN.getPacket(2, &recieved, 1);
   Serial.print(" Recieved: ");
   Serial.println(recieved);
   CAN.send();

   recieved = 0;

   
}