
#include <Arduino.h>
#include <MART_CAN.h>

/*
CAN_BUS CAN(5);


unsigned long removeableIDs[2]={101,103};
void setup()
{
   Serial.begin(9600);
   //Add to don't store permanently any packet in memory
   //CAN->DataIN.addRemovableIds()
   //Add to don't store permanently any packet whose id is in the removeableIDs array
   //CAN->DataIN.addRemovableIds(removeableIDs,2)
}

void loop()
{  
   bool dataBool16[16];
   short dataShort1[1],dataShort2[2];
   int dataInt1[1],dataInt2[2];
   float dataFloat1[1];

   //Se leen los datos del bus y se guardan en memoria (DataIN)
   CAN->receive();
   //Se procesan los primeros 4 bytes del paquete con ID=100 como "int" y se guardan en dataInt1
   CAN->getPacket(100, dataInt1); 

   //Se procesan los 8 bytes del paquete con ID=101 como "int" (de 4 en 4) y se guardan en dataInt2 
   CAN->getPacket(101, dataInt2); 

   //Se procesan los primeros 4 bytes del paquete con ID=102 como "short"(de 2 en 2) y se guardan en dataShort2
   //Los restantes se procesan como "int" y se guardan en dataInt1
   CAN->getPacket(102, dataShort2, dataInt1); 

   //Se procesan los primeros 2 bytes del paquete con ID=200 como "bool"(de bit en bit) y se guardan en dataBool16
   //Los 4 bytes siguientes se procesan como "float" y los dos últimos como "short" 
   CAN->getPacket(200, dataBool16, dataFloat1,dataShort1);

   //getPacket devuelve "false" si se intentan leer más de 8 bytes o si el paquete con la ID buscada no existe
   if(!CAN->getPacket(200, dataBool16, dataFloat1,dataShort1,dataInt1))
   {
      Serial.println("Error. Se están intentando leer más de 8 bytes");
   }

   //Muestra la información almacenada en los arrays por pantalla
   CAN->printArray(dataInt2);
   CAN->printArray(dataBool16);


   //Test receive byteArray
   byte byteArray[8];
   CAN->getPacket((unsigned long)0x12, byteArray);

}
*/

bool flag = false;
unsigned int speed = 1000, speedFactor = 2;
unsigned long tiempoDesdeInicio, tiempoActual = 0;

CAN_BUS* CAN;

int packetID = 100;
const int buttonPin = 25;
unsigned int cont = 0;
byte h[8] = {0,0,0,0,0,0,0,0};
byte h1[8] = {0,0,0,0,0,0,0,0};
byte h2[8] = {0,0,0,0,0,0,0,0};
int dataInt2[2] = {0,0};
void setup(){
   Serial.begin(115200);
   CAN = new CAN_BUS(HardwareType::Controller, speed, 1,10);
   CAN->timeout = 30;
   if(CAN->error == 1){
      Serial.println("Error Initializing ESP32CAN->..");
   }
}



void loop()
{
   
   // Prueba para intentar identificar el error
   
      Serial.println("Antes de CAN->receive()");
      CAN->receive();
      CAN->getPacket(5, h1);
      CAN->getPacket(6, h2);
      CAN->getPacket(7, h);
      CAN->getPacket(2, dataInt2);
      CAN->printArray(h);
      CAN->printArray(h1);
      CAN->printArray(h2);
   CAN->printReceivedIds();


   //Prueba de cambio de velocidad en caliente

   //PRUEBA PARA SEND

   bool dataBool16[16] = {1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1};
   short dataShort1[1] = {12}, dataShort2[2] = {12, -1148};
   int dataInt1[1] = {-3}, dataInt2[2] = {-3, 1237};
   float dataFloat1[1] = {3.142592};
   byte dataByte[8] = {1,0,0,0,1,1,1,1};

   // Se crea un paquete con ID=100 configurado como RRF (petición de datos remota). Los paquetes que estén configurados
   // en los receptores responderán enviando información cuando reciban este paquete
   CAN->setPacket(1);

   // Se procesan los 8 bytes de dataInt2 como "int" (de 4 en 4) y se crea un paquete de ID=101 con esos datos
   CAN->setPacket(2, dataInt2);

   // Se procesan los 4 bytes de dataShort2 como "short"(de 2 en 2), los 4 bytes de dataInt1 como "int" y se crea
   // un paquete de ID=102 con esos datos
   CAN->setPacket(3, dataShort2, dataInt1);

   // Se crea un paquete que contiene información de tipo "bool", "float" y "short"
   CAN->setPacket(4, dataBool16, dataFloat1, dataShort1);

   //Se crea un paquete que contiene informacion de tipo "byte" (8 bytes)
   CAN->setPacket(5, dataByte);

   CAN->send();

   cont++;
}