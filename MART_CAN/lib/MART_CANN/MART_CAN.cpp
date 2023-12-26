#include "MART_CAN.h"

/**
 * Reads a message from the CAN bus if available.
 * It checks for the availability of a message using the interrupt pin.
 * If a message is available, it reads the message ID, length, and data bytes
 * into the DataIN structure. It also determines whether the message uses an
 * extended ID and if it is a Remote Request Frame (RRF).
 * @return true if a message was successfully read, false otherwise.
 */
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

/**
 * Writes a message to the CAN bus using the data in DataOUT structure.
 * It sends a message with the specified message ID, flag for extended ID,
 * message length, and data bytes. The status of the message send operation
 * is checked to ensure successful transmission.
 * @return true if the message was successfully sent, false otherwise.
 */
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

/**
 * Sends all packets in the DataOUT structure to the CAN bus.
 * Iterates over each packet in DataOUT and sends them individually.
 * If a message fails to send, it logs an error but continues sending
 * the remaining messages. The success status is updated accordingly.
 * @return true if all messages were sent successfully, false if any failed.
 */
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

/**
 * Looks for an specific packet in DataOUT and sends it if exists
 * If a message fails to send, it logs an error but continues sending
 * the remaining messages. The success status is updated accordingly.
 * @return true if all messages were sent successfully, false if any failed.
 */
bool CAN_BUS::send(unsigned long id)
{
    bool success = true;

    const CanPacketRawData *packet = DataOUT.getPacketById(id);
    if (packet != nullptr)
    {
        byte len = packet->size;
        byte buf[8];

        // Copy data to buffer
        std::copy(std::begin(packet->bytes), std::end(packet->bytes), std::begin(buf));
        if (_CAN.sendMsgBuf(id, 0, 8, buf) != CAN_OK)
        {
            Serial.println("Error sending message");
            success = false; // Mark failure but continue sending the rest
        }
    }
    else
        success = false;

    return success;
}

/**
 * Receives messages from the CAN bus and stores them in DataIN.
 * This method repeatedly calls readBytes() to read any available CAN messages.
 * Each read message is added to the DataIN structure for later processing.
 * This method is typically called within a loop to continuously read data.
 */
void CAN_BUS::receive()
{
    if (readBytes())
    {

        DataIN.addPacket(DataIN.dataRaw);
        // printArray(DataIN.dataRaw.bytes);
    }
}
