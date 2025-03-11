#include <Arduino.h>
#include <MART_CAN.h>

// MAIN1: PRUEBA PARA 2 ESP32 SIMULTANEAS. ESTE CORRESPONDE A LA QUE TIENE LA ETIQUETA 1
//** CORREGIR2: Recomiendo que las variables que enviáis cambien en tiempo de ejecución para que de verdad podáis observar en el receptor que se están */
//** actualizando los datos. Podéis usar algún contador que se vaya incrementando, usar analogRead() con un potenciómetro, etc... */

//** CORREGIR2: La función send(packetID) envía sólo el paquete con esa ID, sin embargo send() sin argumentos manda todos los que están guardados en memoria de forma automática.
//** una vez que probéis que el send(packetID) por separado funciona, usad la otra función: send() para enviar todo de golpe (llamándola una sóla vez al final del loop() */

#define MAX_BYTES 8

bool flag = false;
unsigned int speed = 500, speedFactor = 2;
unsigned long tiempoDesdeInicio, tiempoActual = 0;
const int buttonPin = 25;
unsigned int cont = 0;
unsigned long packetID;
bool success = false;

CAN_BUS CAN(HardwareType::Transciever, speed, 1);

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

   // LA ESP1 VA A RECIBIR LOS PAQUETES CON ID X Y VA A ENVIAR A LA ESP2 LOS PAQUETES IDX+1

   byte dataByte[MAX_BYTES] = {1,0,0,0,1,1,1,1};
   byte dataReceived[MAX_BYTES];

   for(packetID = 1; packetID <= 10; packetID++)
   {
      if(packetID % 2 != 0)
      {
         CAN.setPacket(packetID, dataByte);
         CAN.printArray(dataByte);
         if(CAN.send(packetID) == true)
         {
            Serial.println("Paquete enviado por ESP1 a ESP2");
         }
      }
      else
      {
         CAN.receive();
         CAN.getPacket(packetID, dataReceived);
         Serial.println("Paquete enviado de ESP2 por ESP1");
         CAN.printReceivedIds();
         CAN.printArray(dataReceived);
      }
      desplazar_derecha(dataByte, MAX_BYTES);
   }

   
   //**CORREGIR2: Para mostrar este mensaje por el monitor serial, se debe de comprobar que el paquete se ha mandado de forma correcta */
   //** para ello podéis usar el bool que devuelve send(), que es true si el paquete se ha enviado OK */


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