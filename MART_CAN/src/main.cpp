
#include <Arduino.h>
#include <MART_CAN.h>

/*
CAN_BUS CAN(5);


unsigned long removeableIDs[2]={101,103};
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
   bool dataBool16[16];
   short dataShort1[1],dataShort2[2];
   int dataInt1[1],dataInt2[2];
   float dataFloat1[1];

   //Se leen los datos del bus y se guardan en memoria (DataIN)
   CAN.receive();
   //Se procesan los primeros 4 bytes del paquete con ID=100 como "int" y se guardan en dataInt1
   CAN.getPacket(100, dataInt1); 

   //Se procesan los 8 bytes del paquete con ID=101 como "int" (de 4 en 4) y se guardan en dataInt2 
   CAN.getPacket(101, dataInt2); 

   //Se procesan los primeros 4 bytes del paquete con ID=102 como "short"(de 2 en 2) y se guardan en dataShort2
   //Los restantes se procesan como "int" y se guardan en dataInt1
   CAN.getPacket(102, dataShort2, dataInt1); 

   //Se procesan los primeros 2 bytes del paquete con ID=200 como "bool"(de bit en bit) y se guardan en dataBool16
   //Los 4 bytes siguientes se procesan como "float" y los dos últimos como "short" 
   CAN.getPacket(200, dataBool16, dataFloat1,dataShort1);

   //getPacket devuelve "false" si se intentan leer más de 8 bytes o si el paquete con la ID buscada no existe
   if(!CAN.getPacket(200, dataBool16, dataFloat1,dataShort1,dataInt1))
   {
      Serial.println("Error. Se están intentando leer más de 8 bytes");
   }

   //Muestra la información almacenada en los arrays por pantalla
   CAN.printArray(dataInt2);
   CAN.printArray(dataBool16);


   //Test receive byteArray
   byte byteArray[8];
   CAN.getPacket((unsigned long)0x12, byteArray);

}
*/

CAN_BUS CAN(HardwareType::Transciever, 500, 1);
   int packetID = 100;
void setup(){
   Serial.begin(9600);

   if(CAN.error == 1){
      Serial.println("Error Initializing ESP32Can...");
   }
}

 void loop(){
   byte h[8];
   //Se leen los datos del bus y se guardan en memoria (DataIN)
   CAN.receive();
   //Se procesan los primeros 4 bytes del paquete con ID=100 como "int" y se guardan en dataInt1
   CAN.getPacket(113, h);
   CAN.printArray(h);
   CAN.printReceivedIds();

   //**CORREGIR: Implementar una pequeña rutina que reinicialice el hardware para cambiar entre las dos velociades del bus (500kbps y 1Mbps)
   //** Esta rutina llamará a setupCANHardware al leer la pulsación de un botón por ejemplo */
}
