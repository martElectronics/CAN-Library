
//*******This sketch sends packets with id 100 and 200 every 5 and 2 seconds respectively

#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS CAN(5);
unsigned long canId = 100;
unsigned long canId2 = 200;
short iShort1[1] = {257};

void setup()
{
  Serial.begin(9600);

  short dataShort1[1] = {12};
  int dataInt1[1] = {-3};

  // Se crean dos paquetes con ID=100 e ID=101
  CAN.setPacket(100, dataShort1);
  CAN.setPacket(101, dataInt1);

  // Se crea un RRF para realizar una petición de datos remota
  CAN.setPacket(200);

  // Se configura el paquete con ID=100 para que sea enviado de manera automática cada 5.5 segundos
  CAN.setPacketTimer(100, 5500);

  // Se configura el paquete con ID=100 para que sea enviado de manera automática cada 2.0 segundos
  CAN.setPacketTimer(101, 2000);

  // Se configura el paquete con ID=200 para realizar una petición de datos remota cada 250 ms
  CAN.setPacketTimer(200, 250);

  //Las únicos mensajes que se recibirán serán aquellos cuyas IDs estén configuradas con setFilters
  unsigned long configIds[4] = {0x101, 0x102, 0x109, 0x110};
  if (CAN.setFilters(configIds, 4))
  {
    Serial.println("Filtros aplicados correctamente");
  }

  //Si se utiliza la configuración anterior, sólo las IDs 0x101, 0x102, 0x109 y 0x110 deberían pasar el test.
  //El resto deben de ser bloqueadas
  std::vector<uint16_t> testIds = {0x101, 0x102, 0x109, 0x110, 0x200, 0x211, 0x500, 0x600, 0x999};

  //Muestra el resultado del test por pantalla
  CAN.testFilters(testIds);
  
}

void loop()
{
  CAN.send();
}
