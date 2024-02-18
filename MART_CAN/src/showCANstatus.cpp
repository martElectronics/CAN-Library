#include <Arduino.h>
#include "MART_CAN.h"

//Esta es una implementación de MART_CAN para mostrar información relevante del bus compartido
//los diferentes nodos.

//***¡NUEVO CONSTRUCTOR! (pinCS, nodeID)
CAN_BUS CAN(5,1);

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  CAN.receive();

  //Muestra por pantalla la información del nodo con ID=1 (este ESP32, ver constructor)
  CAN.printStatusData(1);

  //Muestra por pantalla la información del nodo con ID=3
  CAN.printStatusData(3);

  //Array para almacenar la información del nodo con ID=4
  int node4Data[6]; 
  //Posición 0 - Tiempo de ejecución del loop del nodo 4
  //Posición 1 - Número de paquetes enviados por el nodo 4 que colisionan (por segundo)
  //Posición 2 - Número de paquetes recibidos OK por el nodo 4 (por segundo)
  //Posición 3 - Número de paquetes enviados OK por el nodo 4 (por segundo)
  //Posiciones 4 y 5 - Reservadas
  
  CAN.getCANStatusData(4,node4Data);

  //Ejemplo de uso de los datos del nodo 4
  if(node4Data[0]>=1000)
  {
    //Ejecutar código si el loop() del nodo 4 tarda más de un segundo en completarse
  }

  


  CAN.send();
}



