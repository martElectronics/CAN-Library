#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS can(5);

CanPacketRawData p1, p2;
byte dataBytes[8];

unsigned long ii[1] = {101};

void setup()
{
    dataBytes[0] = 5;
    can.DataOUT.setRemovableIds(ii, 1);

    Serial.begin(9600);
    Serial.println();
    can.config.simulating = true;
    // can.config.autoRemoveRRFPacket=false;

    // Adding some sample RRFIds data
    can.setRRFId(1, 100); // INRRFid: 0x01, OUTRRFid: 0x100
    can.setRRFId(1, 101); // Adding another OUTRRFid to the same INRRFid
    can.setRRFId(2, 100); // A different INRRFid
    can.setRRFId(3, 200); // A different INRRFid

    can.setPacket(100, dataBytes);
    can.setPacket(101, dataBytes);
    can.setPacket(200, dataBytes);
    can.setPacket(22);
    can.setPacket(300, dataBytes);

    p1.id = 1;
    p1.size = 8;
    p1.rrf = true;
    p1.typeExtendedId = false;
    can.DataIN.dataRaw = p1;
    can.receive();

    p2 = p1;
    p2.id = 2;
    can.DataIN.dataRaw = p2;
    can.receive();

    can.DataIN.printAllPackets();

    can.send();
}

void loop()
{
    // Your loop code here
}
