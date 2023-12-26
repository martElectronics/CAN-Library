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
    bool ok;
    if (!digitalRead(_CAN.pinINT) && !config.simulating)
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

        ok = true;
    }
    else
    {
        ok = false;
    }
    return (ok || config.simulating);
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
    bool ok;
    byte sndStat = _CAN.sendMsgBuf(DataOUT.dataRaw.id, DataOUT.dataRaw.typeExtendedId, 8, DataOUT.dataRaw.bytes);
    if (sndStat == CAN_OK)
    {
        ok = true;
    }
    else
    {
        ok = false;
    }
    return (ok || config.simulating);
}

/**
 * Sends all packets in the DataOUT structure to the CAN bus.
 * Iterates over each packet in DataOUT and sends them individually.
 * If a message fails to send, it logs an error but continues sending
 * the remaining messages. The success status is updated accordingly.
 * return true if all messages were sent successfully, false if any failed.
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
        if(!packet.WaitForRRF)
        {
        std::copy(std::begin(packet.bytes), std::end(packet.bytes), std::begin(buf));
        if (_CAN.sendMsgBuf(id, packet.typeExtendedId, 8, buf) != CAN_OK) {
            ERROR_PRINTLN("Error sending message");
            success = false; // Mark failure but continue sending the rest
        }
        else{
            DEBUG_PRINTLN((String)"Packet sent ID = "+id);
        }
        } 
        else
        {
            DEBUG_PRINTLN((String)"Packet not send because is waiting a RRF ID = "+id);
        } }

    );

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
    DEBUG_PRINT((String) "Sending packet with id " + id);
    if (packet != nullptr)
    {
        byte len = packet->size;
        byte buf[8];

        // Copy data to buffer
        std::copy(std::begin(packet->bytes), std::end(packet->bytes), std::begin(buf));
        if (_CAN.sendMsgBuf(id, packet->typeExtendedId, 8, buf) != CAN_OK)
        {
            ERROR_PRINTLN("Error sending message");
            success = false; // Mark failure but continue sending the rest
        }
        else
        {
            DEBUG_PRINTLN(" sent OK");
        }
    }
    else
    {
       DEBUG_PRINTLN("NULLPTR");
        success = false;
    }

    return success;
}

bool CAN_BUS::sendRequestedRRF(unsigned long id)
{
    bool ok = true;
    auto outIds = getOutIdsByInId(id);
    if (outIds)
    {
        DEBUG_PRINTLN("Sending messages for OUTRRFids associated with INRRFid 0x");
        DEBUG_PRINTLN(id);
        for (unsigned long id : outIds.value())
        {
            if (!send(id))
                ok = false; // Call the send method for each OUTRRFid
        }
    }
    else
    {
        ok = false;
       DEBUG_PRINT("No OUTRRFids found for INRRFid 0x");
        DEBUG_PRINTLN(id);
    }
    // returns ok if all the ids that ere config using setRRFId are found and sent correctly
    return (ok || config.simulating);
}

/**
 * Receives messages from the CAN bus and stores them in DataIN.
 * This method repeatedly calls readBytes() to read any available CAN messages.
 * Each read message is added to the DataIN structure for later processing.
 * This method is typically called within a loop to continuously read data.
 */
void CAN_BUS::receive()
{
    if (readBytes() || config.simulating)
    {
        DataIN.addPacket(DataIN.dataRaw);
        // Respond to RRF if the option is enabled
        if (DataIN.dataRaw.rrf && config.respondToRRF)
        {

            DEBUG_PRINTLN("Sending requested paquets of rrf");
            sendRequestedRRF(DataIN.dataRaw.id);
            if (config.autoRemoveRRFPacket)
            {
                DataIN.removePacket(DataIN.dataRaw.id);
            }
        }
    }
}

void CAN_BUS::setRRFId(unsigned long inId, unsigned long outId)
{
    // Search for an existing inId
    auto it = std::find_if(rrfIdsList.begin(), rrfIdsList.end(),
                           [inId](const RRFIds &rrfIds)
                           {
                               return !rrfIds.INRRFid.empty() && rrfIds.INRRFid[0] == inId;
                           });

    if (it != rrfIdsList.end())
    {
        // inId found, append outId to the existing OUTRRFid vector
        it->OUTRRFid.push_back(outId);
    }
    else
    {
        // inId not found, create a new RRFIds instance and insert it
        RRFIds newIds;
        newIds.INRRFid.push_back(inId);
        newIds.OUTRRFid.push_back(outId);
        // Find the correct position to insert to keep the list ordered
        auto insertPos = std::lower_bound(rrfIdsList.begin(), rrfIdsList.end(), inId,
                                          [](const RRFIds &rrfIds, unsigned long id)
                                          {
                                              return !rrfIds.INRRFid.empty() && rrfIds.INRRFid[0] < id;
                                          });
        rrfIdsList.insert(insertPos, newIds);
    }
}

std::optional<std::vector<unsigned long>> CAN_BUS::getOutIdsByInId(unsigned long inId)
{
    for (const auto &rrfIds : rrfIdsList)
    {
        // Check if the inId is in the INRRFid vector
        if (std::find(rrfIds.INRRFid.begin(), rrfIds.INRRFid.end(), inId) != rrfIds.INRRFid.end())
        {
            // If inId is found, return the associated OUTRRFid vector
            return rrfIds.OUTRRFid;
        }
    }
    return std::nullopt; // inId not found
}

bool CAN_BUS::searchOutId(unsigned long outId)
{
    bool ok;
    for (const auto &rrfIds : rrfIdsList)
    {
        // Check if the outId is in the OUTRRFid vector
        if (std::find(rrfIds.OUTRRFid.begin(), rrfIds.OUTRRFid.end(), outId) != rrfIds.OUTRRFid.end())
        {
            ok = true;
        }
        else
        {
            ok = false; // outId not found in any vector
        }
    }
    return (ok);
}