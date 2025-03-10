#include <Arduino.h>
#include <MART_CAN.h>


// MAIN1: PRUEBA PARA 2 ESP32 SIMULTANEAS. ESTE CORRESPONDE A LA QUE TIENE LA ETIQUETA 1
//** CORREGIR2: Recomiendo que las variables que enviáis cambien en tiempo de ejecución para que de verdad podáis observar en el receptor que se están */
//** actualizando los datos. Podéis usar algún contador que se vaya incrementando, usar analogRead() con un potenciómetro, etc... */

//** CORREGIR2: La función send(packetID) envía sólo el paquete con esa ID, sin embargo send() sin argumentos manda todos los que están guardados en memoria de forma automática.
//** una vez que probéis que el send(packetID) por separado funciona, usad la otra función: send() para enviar todo de golpe (llamándola una sóla vez al final del loop() */

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

   // LA ESP1 VA A RECIBIR LOS PAQUETES CON ID X Y VA A ENVIAR A LA ESP2 LOS PAQUETES IDX+1

   bool dataBool16[16] = {1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1};
   short dataShort1[2] = {12, -1148};
   int dataInt1[1] = {-3}, dataInt2[2] = {-3, 1234567};
   byte dataByte[8] = {1,0,0,0,1,1,1,1};

   byte dataReceived[8];

   CAN.setPacket(packetID, dataBool16);
   CAN.printArray(dataBool16);
   CAN.send(packetID);
   //**CORREGIR2: Para mostrar este mensaje por el monitor serial, se debe de comprobar que el paquete se ha mandado de forma correcta */
   //** para ello podéis usar el bool que devuelve send(), que es true si el paquete se ha enviado OK */
   Serial.println("Paquete enviado por ESP1 a ESP2");

   CAN.receive();
   CAN.getPacket(packetID, dataReceived);
   CAN.printArray(dataReceived);
   CAN.printReceivedIds();
   Serial.println("Paquete recibido de ESP2 por ESP1");
   packetID++;

   CAN.setPacket(packetID, dataInt2);
   CAN.printArray(dataInt2);
   CAN.send(packetID);
   Serial.println("Paquete enviado por ESP1 a ESP2");

   CAN.receive();
   CAN.getPacket(packetID, dataReceived);
   CAN.printArray(dataReceived);
   CAN.printReceivedIds();
   Serial.println("Paquete recibido de ESP2 por ESP1");
   packetID++;

   CAN.setPacket(packetID, dataShort1, dataInt1);
   CAN.send(packetID);
   Serial.println("Paquete enviado por ESP1 a ESP2");

   CAN.receive();
   CAN.getPacket(packetID, dataReceived);
   CAN.printArray(dataReceived);
   CAN.printReceivedIds();
   Serial.println("Paquete recibido de ESP2 por ESP1");
   packetID++;

   CAN.setPacket(packetID, dataByte);
   CAN.printArray(dataByte);
   CAN.send(packetID);
   Serial.println("Paquete enviado por ESP1 a ESP2");


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