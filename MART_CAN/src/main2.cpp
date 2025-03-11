#include <Arduino.h>
#include <MART_CAN.h>


// MAIN2: PRUEBA PARA 2 ESP32 SIMULTANEAS. ESTE CORRESPONDE A LA QUE NO TIENE LA ETIQUETA

#define MAX_BYTES 8

bool flag = false, fin = false;
unsigned int speed = 500, speedFactor = 2;
unsigned long tiempoDesdeInicio, tiempoActual = 0;
const int buttonPin = 25;
unsigned int cont = 0;
unsigned long packetID;
bool success = true;

CAN_BUS CAN(HardwareType::Transciever, speed, 1);
//CAN_BUS CAN(HardwareType::Transciever, speed, 1, 4, 5);

void desplazar_derecha(byte dataByte[], int size) 
{
   byte ultimo = dataByte[size - 1];

   for (int i = size - 1; i > 0; i--) 
   {
       dataByte[i] = dataByte[i - 1];
   }

   dataByte[0] = ultimo;
}

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

   byte dataByte[MAX_BYTES] = {1,0,0,0,1,1,1,1};
   byte dataReceived[MAX_BYTES];

   // PARA COMPROBAR IDs NORMALES (<= 2047) CON SEND(ID)
   for(packetID = 1; packetID <= 200; packetID++)
   {
      if(packetID % 2 == 0)
      {
         CAN.setPacket(packetID, dataByte);
         Serial.print("\nPaquete para enviar: ");fflush(stdout);
         CAN.printArray(dataByte);
         success = CAN.send(packetID);
         if(success == true)
         {
            Serial.println("Paquete enviado por ESP2 a ESP1\n");fflush(stdout);
            desplazar_derecha(dataByte, MAX_BYTES);
         }
      }
      else
      {
         CAN.receive();
         CAN.getPacket(packetID, dataReceived);
         Serial.println("Paquete recibido de ESP1 por ESP2");fflush(stdout);
         Serial.print("Paquete recibido: ");fflush(stdout);
         CAN.printArray(dataReceived);
      }
   }
   
   // PARA COMPROBAR IDs EXTENDIDAS (>2047) CON SEND(ID)
   /*for(packetID = 3000; packetID <= 3200; packetID++)
   {
      if(packetID % 2 == 0)
      {
         CAN.setPacket(packetID, dataByte);
         Serial.print("\nPaquete para enviar: ");fflush(stdout);
         CAN.printArray(dataByte);
         success = CAN.send(packetID);
         if(success == true)
         {
            Serial.println("Paquete enviado por ESP2 a ESP1\n");fflush(stdout);
            desplazar_derecha(dataByte, MAX_BYTES);
         }
      }
      else
      {
         CAN.receive();
         CAN.getPacket(packetID, dataReceived);
         Serial.println("Paquete recibido de ESP1 por ESP2");fflush(stdout);
         Serial.print("Paquete recibido: ");fflush(stdout);
         CAN.printArray(dataReceived);
      }
   }*/

   // PARA PROBAR SEND()
   /*for(packetID = 3000; packetID <= 3200; packetID++)
   {
      if(packetID % 2 == 0)
      {
         CAN.setPacket(packetID, dataByte);
         Serial.print("Paquete para enviar: ");fflush(stdout);
         CAN.printArray(dataByte);
         desplazar_derecha(dataByte, MAX_BYTES);
      }
   }

   success = CAN.send();
   if(success == true)
   {
      Serial.print(packetID);
      Serial.println("paquetes enviados por ESP1 a ESP2\n");fflush(stdout);
   }

   for(packetID = 3000; packetID <= 3200; packetID++)
   {
      if(packetID % 2 != 0)
      {
         CAN.receive();
         CAN.getPacket(packetID, dataReceived);
      }
   }

   Serial.println("Paquetes recibidos de ESP2 por ESP1:");fflush(stdout);
   CAN.printReceivedIds();*/


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