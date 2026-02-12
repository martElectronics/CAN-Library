#include "MART_CAN.h"

/**
 * Reads a message from the CAN bus if available.
 * It checks for the availability of a message using the interrupt pin.
 * If a message is available, it reads the message ID, length, and data bytes
 * into the DataIN structure. It also determines whether the message uses an
 * extended ID and if it is a Remote Request Frame (RRF).
 * return true if a message was successfully read, false otherwise.
 */
bool CAN_BUS::readBytes()
{
    bool ok = false;
    if (type == HardwareType::Controller)
    {
        // if (!digitalRead(_CAN.pinINT) && !config.simulating)
        // {
        //     // byte rxBuf[8];
        //     // _CAN.readMsgBuf(&DataIN.dataRaw.id, &DataIN.dataRaw.size, DataIN.dataRaw.bytes); // Read data: len = data length, buf = data byte(s)
        //     // if ((DataIN.dataRaw.id & 0x80000000) == 0x80000000)
        //     //     DataIN.dataRaw.typeExtendedId = true;
        //     // else
        //     //     DataIN.dataRaw.typeExtendedId = false;

        //     // if ((DataIN.dataRaw.id & 0x40000000) == 0x40000000)
        //     // {
        //     //     unsigned long mask = ~(1UL << 30);
        //     //     // Clear the bit at bitPosition
        //     //     DataIN.dataRaw.id &= mask;
        //     //     DataIN.dataRaw.rrf = true;
        //     //     DEBUG_PRINTLN("Received RRF");
        //     // }
        //     // else
        //     //     DataIN.dataRaw.rrf = false;

        //     // ok = true;
        // }
        // else
        // {
        //     ok = false;
        // }
    }
    else if (type == HardwareType::Transciever)
    {
#if defined(ARDUINO_MICRO)
        Serial.println("Not available for arduino + transciever");
#endif
#if defined(ESP32) || defined(ESP32S3)

        CanFrame frame = {0};
        if (ESP32Can.readFrame(frame) && !config.simulating)
        {
            DataIN.dataRaw.id = frame.identifier;
            DataIN.dataRaw.size = frame.data_length_code;
            for (int i = 0; i < 8; i++)
            {
                DataIN.dataRaw.bytes[i] = frame.data[i];
            }
            DataIN.dataRaw.typeExtendedId = frame.extd;

            ok = true;
        }
#endif
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
    // bool ok;
    // byte sndStat = _CAN.sendMsgBuf(DataOUT.dataRaw.id, DataOUT.dataRaw.typeExtendedId, 8, DataOUT.dataRaw.bytes);
    // if (sndStat == CAN_OK)
    // {
    //     ok = true;
    // }
    // else
    // {
    //     ok = false;
    // }
    // return (ok || config.simulating);
    return 0;
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

    unsigned long currentTime = millis();

    // Send status data if the timer reaches PT and the ESP is configured accordingly
    if (((millis() - previousStatusIntervalTime) >= (intervalTime / 3)) && (config.sendStatusData))
    {
        numCurrentSamples++;
        setCANStatusData();
        previousStatusIntervalTime = millis();

        if (numCurrentSamples > 3)
        {
            // numRXPaqOK = 0;
            // numTXPaqOK = 0;
            // numTxPaqError = 0;
            // numCurrentSamples = 1;
        }
    }

    DataOUT.forEachPacket([this, &success, currentTime](CanPacketRawData &packet)
                          {
        // Initially assume the packet does not have a timer and is ready to send
        bool readyToSend = true;

        // Check for a timer associated with this packet
        for (const auto& timer : packetTimers) {
            if (timer.packetID == packet.id) {
                // Check if the current time is past the next scheduled send time
                if (currentTime < packet.nextSendTime) {
                    readyToSend = false; // Not yet time to send this packet
                }
                break; // Timer found, no need to check further
            }
        }

       
         
        if (readyToSend) {

            //Store the packet ID before the possible ID change if the packet is a RRF
            unsigned long idAux=packet.id;
            byte buf[8];
            // Copy data to buffer
            
            memcpy(buf, packet.bytes, packet.size);

            // Attempt to send the packet
            if(type == HardwareType::Controller){
            //     if (_CAN.sendMsgBuf(packet.id, packet.size, buf) != CAN_OK) {
                    
                
            //         ERROR_PRINTLN("Error sending message");
            //         success = false; // Mark failure but continue sending the rest
            //         numTxPaqError++;
            //     } else {
            //   DEBUG_PRINTLN((String)"Packet sent ID = " + packet.id);
            //     packet.id=idAux;
            //     // Update the next send time for this packet if it has a timer
            //     for (auto& timer : packetTimers) {
            //         if (timer.packetID == packet.id) {
                        
            //             packet.nextSendTime = currentTime + timer.interval;
            //             break;
            //         }
            //     }
            //     numTXPaqOK++;
            // }
            }
            else if(type == HardwareType::Transciever){
#if defined(ARDUINO_MICRO)
                Serial.println("Not available for arduino + transciever");
#endif
#if defined(ESP32) || defined(ESP32S3)
                
                CanFrame frame = {0};
                frame.identifier = packet.id;
                frame.extd = packet.typeExtendedId;
                frame.data_length_code = packet.size;
                for (int i = 0; i < 8; i++)
                {
                    frame.data[i] = packet.bytes[i];
                }
                if (!ESP32Can.writeFrame(frame,0))
                {
                    ERROR_PRINTLN("Error sending message");
                    success = false; // Mark failure but continue sending the rest
                    numTxPaqError++;
                }
                else
                {
                    DEBUG_PRINTLN((String)"Packet sent ID = " + packet.id);
                    packet.id=idAux;
                    // Update the next send time for this packet if it has a timer
                    for (auto& timer : packetTimers) {
                        if (timer.packetID == packet.id) {
                            packet.nextSendTime = currentTime + timer.interval;
                            break;
                        }
                    }
                    numTXPaqOK++;
                }
#endif
            }
            }
           }); // Ensure this closing brace matches the lambda function

    runtimeTime = millis() - previousStatusRuntimeTime;

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
        memcpy(buf, packet->bytes, len);
        if (type == HardwareType::Controller)
        {
            // if (_CAN.sendMsgBuf(packet->id, packet->size, buf))
            // {
            //     ERROR_PRINTLN("Error sending message");
            //     success = false; // Mark failure but continue sending the rest
            // }
            // else
            // {
            //     DEBUG_PRINTLN(" sent OK");
            // }
        }
        else if (type == HardwareType::Transciever)
        {
#if defined(ARDUINO_MICRO)
            Serial.println("Not available for arduino + transciever");
#endif
#if defined(ESP32) || defined(ESP32S3)

            CanFrame frame = {0};
            frame.identifier = packet->id;
            frame.extd = packet->typeExtendedId;
            frame.data_length_code = packet->size;
            for (int i = 0; i < 8; i++)
            {
                frame.data[i] = packet->bytes[i];
            }
            if (!ESP32Can.writeFrame(frame))
            {
                ERROR_PRINTLN("Error sending message");
                success = false; // Mark failure but continue sending the rest
            }
#endif

#if defined(ARDUINO_MICRO)
            Serial.println("Not available for arduino + transciever");
#endif
        }
    }
    else
    {
        DEBUG_PRINTLN("NULLPTR");
        success = false;
    }

    return success;
}

/**
 * Receives messages from the CAN bus and stores them in DataIN.
 * This method repeatedly calls readBytes() to read any available CAN messages.
 * Each read message is added to the DataIN structure for later processing.
 */
void CAN_BUS::receive()
{
    previousStatusRuntimeTime = millis();
    int after;
    if (readBytes() || config.simulating)
    {
        int mills = millis();
        DEBUG_PRINTLN((String) "Time to read packet: " + (mills - previousStatusRuntimeTime));
        // Store packet in memory if is not in the IDs set by the filter or if are no ids stored
        if ((filterIDs.empty()) || (std::binary_search(filterIDs.begin(), filterIDs.end(), DataIN.dataRaw.id)))
        {
            // Serial.println("ADDED");
            DataIN.addPacket(DataIN.dataRaw);
        }
        after = millis();
        DEBUG_PRINTLN((String) "Time to add packet: " + (after - mills));
        mills = after;
        DEBUG_PRINTLN((String) "Rx ID: " + DataIN.dataRaw.id);
        
        numRXPaqOK++;
    }
}

// Calculates and writes the masks and filters to the MCP2515 registers given a set of IDs
bool CAN_BUS::setFilters(const unsigned long ids[], unsigned size)
{
    for (int i = 0; i < size; i++)
    {
        filterIDs.push_back(ids[i]);
    }
    std::sort(filterIDs.begin(), filterIDs.end());
    return true;
}

void CAN_BUS::printFilters()
{
    // configurator.printCalculatedValues();
}

void CAN_BUS::testFilters(const std::vector<uint16_t> &testIds)
{
    // configurator.testFilters(testIds);
}

void CAN_BUS::setPacketTimer(unsigned long packetID, unsigned long time)
{
    // Check if the timer already exists and update it
    for (auto &timer : packetTimers)
    {
        if (timer.packetID == packetID)
        {
            timer.interval = time;
            return;
        }
    }

    // If not found, add a new timer
    packetTimers.push_back({packetID, time});
}

//** CAN STATUS **//
void CAN_BUS::setCANStatusData()
{
    int d0[2]; // runtimeTime,numTxPaqError
    int d1[2]; // numRXPaqOK,numTXPaqOK
    int d2[2] = {0, 0};
    d0[0] = runtimeTime;
    d0[1] = numTxPaqError / numCurrentSamples;
    d1[0] = numRXPaqOK / numCurrentSamples;
    d1[1] = numTXPaqOK / numCurrentSamples;

    DEBUG_PRINTLN("Offset: ");
    DEBUG_PRINTLN(statusPacketOffset);

    // this->setPacket(statusPacketOffset, d0);
    // this->setPacket(statusPacketOffset + 1, d1);
    // this->setPacket(statusPacketOffset + 2, d2);
}

void CAN_BUS::printReceivedIds()
{
    DataIN.printAllPacketsIDs();
}

bool CAN_BUS::rebootBusFromError()
{
    twai_status_info_t status;

    if (status.state != TWAI_STATE_BUS_OFF) {
        return true;
    }

    twai_initiate_recovery();
    delay(10);

    //Comprobar si se ha recuperado
    twai_get_status_info(&status);
    if (status.state != TWAI_STATE_BUS_OFF) {
                return true;
    }

    return false; // no recuperado

}

/**
 * Configura los temporizadores de envío de paquetes según su prioridad inversa (ID).
 * IDs más bajos (alta prioridad) se enviarán con intervalos más largos,
 * para no monopolizar el bus.
 */
void CAN_BUS::configurePacketTimersByPriority()
{
    const unsigned long ids[] = {
        10, 11, 12, 33, 65, 97, 129, 161, 193, 225, 257, 289, 321,
        353, 385, 386, 387, 388, 389, 390, 391, 392, 393, 400,
        1025, 1057, 1089, 1121, 1153, 1160, 1161, 1162, 1163,
        1164, 1165, 1166, 1167, 1168, 1169, 1170
    };

    const size_t numIds = sizeof(ids) / sizeof(ids[0]);
    const unsigned long minInterval = 50;    // ms
    const unsigned long maxInterval = 800;   // ms

    // Encontrar el ID mínimo y máximo reales
    unsigned long minId = ids[0];
    unsigned long maxId = ids[0];
    for (size_t i = 1; i < numIds; ++i) {
        if (ids[i] < minId) minId = ids[i];
        if (ids[i] > maxId) maxId = ids[i];
    }

    for (size_t i = 0; i < numIds; ++i) {
        // Escalar según el valor de ID
        float ratio = static_cast<float>(ids[i] - minId) / (maxId - minId);
        ratio = 1.0f - ratio;

        unsigned long interval = minInterval + static_cast<unsigned long>(ratio * (maxInterval - minInterval));

        setPacketTimer(ids[i], interval);

        DEBUG_PRINTLN((String)"[TIMER] ID " + ids[i] + " -> " + interval + " ms");
    }
}