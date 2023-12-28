
//*******This sketch simulates a rrf request and send the corresponding data (only for simulation purposes)
#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS CAN(5);

CanPacketRawData p1, p2;
byte dataBytes[8];

unsigned long ii[1] = {101};

void setup()
{
    dataBytes[0] = 5;
    CAN.DataOUT.setRemovableIds(ii, 1);

    Serial.begin(9600);
    Serial.println();
    CAN.config.simulating = true;
    // CAN.config.autoRemoveRRFPacket=false;

    ///Configuración de las asociaciaciones de paquetes RRF (petición de datos remota)

    //Cuando se reciba un paquete RRF con ID=1, se enviarán los paquetes con ID=100 e ID=101 automaticamente 
    //si están guardados en memoria
    CAN.setRRFId(1, 100);
    CAN.setRRFId(1, 101); 

    //Cuando se reciba un paquete RRF con ID=200, ID=201 o ID=202, se enviará el paquete con ID=301 automaticamente 
    //si está guardado en memoria
    CAN.setRRFId(200, 301); 
    CAN.setRRFId(201, 301); 
    CAN.setRRFId(202, 301); 



    CAN.setPacket(100, dataBytes);
    CAN.setPacket(101, dataBytes);
    CAN.setPacket(200, dataBytes);
    CAN.setPacket(22);
    CAN.setPacket(300, dataBytes);

    p1.id = 1;
    p1.size = 8;
    p1.rrf = true;
    p1.typeExtendedId = false;
    CAN.DataIN.dataRaw = p1;
    CAN.receive();

    p2 = p1;
    p2.id = 2;
    CAN.DataIN.dataRaw = p2;
    CAN.receive();

    CAN.DataIN.printAllPackets();

    CAN.send();


}

void loop()
{
    // Your loop code here
}
