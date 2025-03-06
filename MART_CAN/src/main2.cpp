#include <Arduino.h>
#include <MART_CAN.h>


// MAIN2: PRUEBA PARA 2 ESP32 SIMULTANEAS. ESTE CORRESPONDE A LA QUE NO TIENE LA ETIQUETA


bool flag = false;
unsigned int speed = 500, speedFactor = 2;
unsigned long tiempoDesdeInicio, tiempoActual = 0;
const int buttonPin = 25;
unsigned int cont = 0;
unsigned long packetID = 1;

CAN_BUS CAN(HardwareType::Transciever, speed, 1);

int checkButton()
{
   return digitalRead(buttonPin);
}

void setup()
{
   Serial.begin(115200);

   if(CAN.error == 1)
   {
      Serial.println("Error Initializing ESP32Can...");
   }
   pinMode(buttonPin, INPUT_PULLUP);
}

void loop()
{
   tiempoDesdeInicio = millis();

   // LA ESP2 VA A RECIBIR LOS PAQUETES CON ID X Y VA A ENVIAR A LA ESP1 LOS PAQUETES IDX+1

   bool dataBool16[16] = {1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1};
   short dataShort1[2] = {12, -1148};
   int dataInt1[1] = {-3}, dataInt2[2] = {-3, 1234567};
   byte dataByte[8] = {1,0,0,0,1,1,1,1};

   byte dataReceived[8];

   CAN.setPacket(packetID, dataBool16);
   CAN.printArray(dataBool16);
   CAN.send(packetID);
   Serial.println("Paquete enviado por ESP2 a ESP1");


   CAN.receive();
   CAN.getPacket(packetID, dataReceived);
   CAN.printArray(dataReceived);
   CAN.printReceivedIds();
   Serial.println("Paquete recibido de ESP1 por ESP2");
   packetID++;

   CAN.setPacket(packetID, dataInt2);
   CAN.printArray(dataInt2);
   CAN.send(packetID);
   Serial.println("Paquete enviado por ESP2 a ESP1");

   CAN.receive();
   CAN.getPacket(packetID, dataReceived);
   CAN.printArray(dataReceived);
   CAN.printReceivedIds();
   Serial.println("Paquete recibido de ESP1 por ESP2");
   packetID++;

   CAN.setPacket(packetID, dataShort1, dataInt1);
   CAN.send(packetID);
   Serial.println("Paquete enviado por ESP2 a ESP1");

   CAN.receive();
   CAN.getPacket(packetID, dataReceived);
   CAN.printArray(dataReceived);
   CAN.printReceivedIds();
   Serial.println("Paquete recibido de ESP1 por ESP2");
   packetID++;

   CAN.setPacket(packetID, dataByte);
   CAN.printArray(dataByte);
   CAN.send(packetID);
   Serial.println("Paquete enviado por ESP2 a ESP1");


   //Cada 100ms se consulta el estado del pulsador mientras se ejecutan las demás tareas
   if(tiempoDesdeInicio - tiempoActual >= 100)
   {
      tiempoActual = tiempoDesdeInicio;

      if(checkButton() == LOW && flag == false) // Si está pulsado y no ha sido pulsado antes se cambia la velocidad
      {
         CAN.setupCANHardware(speedFactor*speed);
         flag = true;
         Serial.print("Se ha cambiado la velocidad: ");
         Serial.println(speedFactor*speed);
      }
      else if(checkButton() == HIGH && flag == true) // Si no está pulsado y se ha cambiado antes la velocidad, se cambia
      {                                              // el factor para que la siguiente vez alterne la velocidad
         flag = false;
         speedFactor = (speedFactor % 2) + 1;
      }
   }
   cont++;
}