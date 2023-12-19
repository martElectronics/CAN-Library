#include "MART_CAN.h"

bool CAN_BUS::readBytes()
{
    if (!digitalRead(_CAN.pinINT))
    {
        byte rxBuf[8];
        _CAN.readMsgBuf(&DataIN.dataRaw.id, &DataIN.dataRaw.size, DataIN.dataRaw.bytes); // Read data: len = data length, buf = data byte(s)
        if ((DataIN.dataRaw.id & 0x80000000) == 0x80000000)
            DataIN.dataRaw.typeExtendedId = true;
        else
            DataIN.dataRaw.typeExtendedId = false;

        if ((DataIN.dataRaw.id & 0x40000000) == 0x40000000)
            DataIN.dataRaw.rrf = true;
        else
            DataIN.dataRaw.rrf = false;

        return true;
    }
    return false;
}

bool CAN_BUS::writeBytes()
{
    byte sndStat = _CAN.sendMsgBuf(DataOUT.dataRaw.id, DataOUT.dataRaw.typeExtendedId, 8, DataOUT.dataRaw.bytes);
    if (sndStat == CAN_OK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CAN_BUS::send()
{
    bool success = true;

    // Use forEachPacket to iterate over all packets in DataOUT
    DataOUT.forEachPacket([this, &success](const CanPacketRawData &packet)
                          {
        long unsigned int id = packet.id;
        byte len = packet.size;
        byte buf[8];

        // Copy data to buffer
        std::copy(std::begin(packet.bytes), std::end(packet.bytes), std::begin(buf));
        if (_CAN.sendMsgBuf(id, 0, 8, buf) != CAN_OK) {
            Serial.println("Error sending message");
            success = false; // Mark failure but continue sending the rest
        } });

    return success;
}

void CAN_BUS::receive()
{
    if (readBytes())
    {

        DataIN.addPacket(DataIN.dataRaw);
        // printArray(DataIN.dataRaw.bytes);
    }
}
