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
    if (type == HardwareType::Transciever)
    {
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
#elif defined(STM32G4xx)
        if (HAL_FDCAN_GetRxFifoFillLevel(&hfdcan, FDCAN_RX_FIFO0) > 0 && !config.simulating)
        {
            // GetRxMessage solo copia DLC bytes: sin esto, los que falten serian
            // los de la trama anterior (de cualquier ID).
            memset(DataIN.dataRaw.bytes, 0, sizeof(DataIN.dataRaw.bytes));
            if (HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &RxHeader, DataIN.dataRaw.bytes) == HAL_OK)
            {
                DataIN.dataRaw.id = RxHeader.Identifier;
                
                // Map DLC to size
                switch (RxHeader.DataLength) {
                    case FDCAN_DLC_BYTES_0: DataIN.dataRaw.size = 0; break;
                    case FDCAN_DLC_BYTES_1: DataIN.dataRaw.size = 1; break;
                    case FDCAN_DLC_BYTES_2: DataIN.dataRaw.size = 2; break;
                    case FDCAN_DLC_BYTES_3: DataIN.dataRaw.size = 3; break;
                    case FDCAN_DLC_BYTES_4: DataIN.dataRaw.size = 4; break;
                    case FDCAN_DLC_BYTES_5: DataIN.dataRaw.size = 5; break;
                    case FDCAN_DLC_BYTES_6: DataIN.dataRaw.size = 6; break;
                    case FDCAN_DLC_BYTES_7: DataIN.dataRaw.size = 7; break;
                    default: DataIN.dataRaw.size = 8; break;
                }
                
                DataIN.dataRaw.typeExtendedId = (RxHeader.IdType == FDCAN_EXTENDED_ID);
                ok = true;
            }
        }
#endif
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

            if(type == HardwareType::Transciever){
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
#elif defined(STM32G4xx)
                TxHeader.Identifier = packet.id;
                TxHeader.IdType = packet.typeExtendedId ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
                TxHeader.TxFrameType = FDCAN_DATA_FRAME;
                
                switch(packet.size) {
                    case 0: TxHeader.DataLength = FDCAN_DLC_BYTES_0; break;
                    case 1: TxHeader.DataLength = FDCAN_DLC_BYTES_1; break;
                    case 2: TxHeader.DataLength = FDCAN_DLC_BYTES_2; break;
                    case 3: TxHeader.DataLength = FDCAN_DLC_BYTES_3; break;
                    case 4: TxHeader.DataLength = FDCAN_DLC_BYTES_4; break;
                    case 5: TxHeader.DataLength = FDCAN_DLC_BYTES_5; break;
                    case 6: TxHeader.DataLength = FDCAN_DLC_BYTES_6; break;
                    case 7: TxHeader.DataLength = FDCAN_DLC_BYTES_7; break;
                    default: TxHeader.DataLength = FDCAN_DLC_BYTES_8; break;
                }
                
                TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
                TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
                TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
                TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
                TxHeader.MessageMarker = 0;

                if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &TxHeader, packet.bytes) != HAL_OK)
                {
                    ERROR_PRINTLN("Error sending message");
                    success = false;
                    numTxPaqError++;
                }
                else
                {
                    DEBUG_PRINTLN((String)"Packet sent ID = " + packet.id);
                    packet.id=idAux;
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

    CanPacketRawData packet;
    DEBUG_PRINT((String) "Sending packet with id " + id);
    if (DataOUT.getPacketById(id, packet))
    {
        byte len = packet.size;
        byte buf[8];

        // Copy data to buffer
        memcpy(buf, packet.bytes, len);
        if (type == HardwareType::Transciever)
        {
#if defined(ESP32) || defined(ESP32S3)

            CanFrame frame = {0};
            frame.identifier = packet.id;
            frame.extd = packet.typeExtendedId;
            frame.data_length_code = packet.size;
            for (int i = 0; i < 8; i++)
            {
                frame.data[i] = packet.bytes[i];
            }
            if (!ESP32Can.writeFrame(frame))
            {
                ERROR_PRINTLN("Error sending message");
                success = false; // Mark failure but continue sending the rest
            }
#elif defined(STM32G4xx)
            TxHeader.Identifier = packet.id;
            TxHeader.IdType = packet.typeExtendedId ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
            TxHeader.TxFrameType = FDCAN_DATA_FRAME;
            
            switch(packet.size) {
                case 0: TxHeader.DataLength = FDCAN_DLC_BYTES_0; break;
                case 1: TxHeader.DataLength = FDCAN_DLC_BYTES_1; break;
                case 2: TxHeader.DataLength = FDCAN_DLC_BYTES_2; break;
                case 3: TxHeader.DataLength = FDCAN_DLC_BYTES_3; break;
                case 4: TxHeader.DataLength = FDCAN_DLC_BYTES_4; break;
                case 5: TxHeader.DataLength = FDCAN_DLC_BYTES_5; break;
                case 6: TxHeader.DataLength = FDCAN_DLC_BYTES_6; break;
                case 7: TxHeader.DataLength = FDCAN_DLC_BYTES_7; break;
                default: TxHeader.DataLength = FDCAN_DLC_BYTES_8; break;
            }
            
            TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
            TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
            TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
            TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
            TxHeader.MessageMarker = 0;

            if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &TxHeader, buf) != HAL_OK)
            {
                ERROR_PRINTLN("Error sending message");
                success = false;
            }
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

// Filtro software (receive() solo guarda estos IDs) y, en STM32, tambien HW
bool CAN_BUS::setFilters(const unsigned long ids[], unsigned size)
{
    for (unsigned i = 0; i < size; i++)
    {
        filterIDs.push_back(ids[i]);
    }
    std::sort(filterIDs.begin(), filterIDs.end());
    filterIDs.erase(std::unique(filterIDs.begin(), filterIDs.end()), filterIDs.end());
#if defined(STM32G4xx)
    if (type == HardwareType::Transciever)
    {
        if (error != 0)
            return false;
        return applySTM32FDCANFilterIds();
    }
#endif
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
#if defined(ESP32) || defined(ESP32S3)
    twai_status_info_t status;
    twai_get_status_info(&status);

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
#elif defined(STM32G4xx)
    // El estado bus-off vive en el Protocol Status Register, NO en el ErrorCode
    // del HAL. HAL_FDCAN_GetError() devuelve los HAL_FDCAN_ERROR_* (otro dominio):
    // enmascararlo con FDCAN_PSR_BO daba 0 casi siempre -> la recuperación nunca
    // se ejecutaba. La fuente correcta es GetProtocolStatus().BusOff.
    FDCAN_ProtocolStatusTypeDef psr;
    HAL_FDCAN_GetProtocolStatus(&hfdcan, &psr);
    if (psr.BusOff == 0) {
        return true; // No estamos en bus-off
    }

    // En bus-off el FDCAN pone INIT=1 él solo; al limpiarlo arranca la
    // secuencia de recuperación (128 x 11 bits recesivos, ~11 ms a 125 kbps).
    // No se espera aquí: con el delay(10) de antes la comprobación salía
    // siempre en falso (la recuperación tarda más) y el loop del llamante se
    // frenaba 10 ms por vuelta mientras durase el bus-off. BusOff sigue a 1
    // hasta que la recuperación termina; las siguientes llamadas lo verán.
    if (READ_BIT(hfdcan.Instance->CCCR, FDCAN_CCCR_INIT)) {
        CLEAR_BIT(hfdcan.Instance->CCCR, FDCAN_CCCR_INIT);
    }
    return false; // recuperación en curso
#endif
}

/**
 * Configura los temporizadores de envío de paquetes según su prioridad inversa (ID).
 * IDs más bajos (alta prioridad) se enviarán con intervalos más largos,
 * para no monopolizar el bus.
 */
void CAN_BUS::configurePacketTimersByPriority()
{
    const unsigned long ids[] = {
        10, 11, 12, 13, 14, 15, 16, 33, 65, 97, 129, 161, 193, 225, 257, 289, 321,
        353, 385, 386, 387, 388, 389, 390, 391, 392, 393,
        1025, 1057, 1089, 1121, 1153, 1160, 1161, 1162, 1163,
        1164, 1165, 1166, 1167
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

#if defined(STM32G4xx)
// Busca un prescaler y un numero de time quanta (TQ) que den EXACTAMENTE el
// bitrate pedido con el reloj real del FDCAN. Se prueba de 20 a 8 TQ: con los
// 24 MHz de la Nucleo salen los mismos valores que la tabla fija de antes
// (16 TQ a 125/250/500k, 12 TQ a 1M). Sample point ~80%.
static bool computeNominalTiming(uint32_t clk, uint32_t bitrate,
                                 uint32_t &presc, uint32_t &seg1, uint32_t &seg2)
{
    if (clk == 0 || bitrate == 0)
        return false;
    for (uint32_t tq = 20; tq >= 8; --tq)
    {
        uint32_t div = bitrate * tq;
        if (clk % div != 0)
            continue;
        uint32_t p = clk / div;
        if (p > 512)
            continue;
        presc = p;
        seg2 = (tq / 5 > 2) ? tq / 5 : 2;   // 16 TQ -> 3 (81.25%), 12 TQ -> 2 (83.3%)
        seg1 = tq - 1 - seg2;
        return true;
    }
    return false;
}

HAL_StatusTypeDef CAN_BUS::initSTM32FDCAN(unsigned int speed)
{
    // Enable GPIO and FDCAN clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_FDCAN_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // RX on PA11, TX on PA12 (Standard Nucleo G474RE pins)
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hfdcan.Instance = FDCAN1;

    // Formula: Baudrate = FDCAN_CLK / (Prescaler * (1 + TimeSeg1 + TimeSeg2))
    // FDCAN_CLK se lee del arbol de relojes en vez de suponerlo: en la Nucleo
    // G474RE es el HSE de 24 MHz (valor de reset de FDCANSEL), pero en otra
    // placa, con otro cristal u otra fuente, el bitrate saldria mal sin avisar.
    // Si no hay un timing exacto para ese reloj, el init falla (SetupState != 0).
    hfdcan.Init.ClockDivider = FDCAN_CLOCK_DIV1;
    hfdcan.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
    hfdcan.Init.Mode = FDCAN_MODE_NORMAL;
    hfdcan.Init.AutoRetransmission = ENABLE;
    hfdcan.Init.TransmitPause = DISABLE;
    hfdcan.Init.ProtocolException = DISABLE;

    uint32_t fdcanClk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_FDCAN);
    uint32_t presc, seg1, seg2;
    if (!computeNominalTiming(fdcanClk, (uint32_t)speed * 1000UL, presc, seg1, seg2))
    {
        Serial.println("FDCAN: no hay bit timing exacto para ese bitrate con el reloj actual");
        return HAL_ERROR;
    }
    hfdcan.Init.NominalPrescaler = presc;
    hfdcan.Init.NominalTimeSeg1 = seg1;
    hfdcan.Init.NominalTimeSeg2 = seg2;
    // SJW = TSEG2: maxima tolerancia a la diferencia de reloj con los otros
    // nodos (con SJW=1 cada bit solo se podia corregir 1 TQ).
    hfdcan.Init.NominalSyncJumpWidth = seg2;

    // ExtBitTime
    hfdcan.Init.DataPrescaler = 1;
    hfdcan.Init.DataSyncJumpWidth = 1;
    hfdcan.Init.DataTimeSeg1 = 1;
    hfdcan.Init.DataTimeSeg2 = 1;

    // Todos los del G4, para que setFilters() pueda programar tantos IDs como
    // quepan. HAL_FDCAN_Init pone la RAM de mensajes a 0, y un elemento a 0
    // esta deshabilitado: los que no configura el perfil no filtran nada.
    hfdcan.Init.StdFiltersNbr = STM32_FDCAN_STD_FILTERS;
    hfdcan.Init.ExtFiltersNbr = 0;
    hfdcan.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;

    if (HAL_FDCAN_Init(&hfdcan) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Configure standard filter based on profile
    if (configSTM32FDCANFilter(filterProfile) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // FIFO llena -> se sobrescribe la trama MAS ANTIGUA. Con tramas periodicas
    // (temperaturas, estados) interesa la mas reciente; en el modo por defecto
    // (blocking) se descartaban las nuevas.
    if (HAL_FDCAN_ConfigRxFifoOverwrite(&hfdcan, FDCAN_RX_FIFO0, FDCAN_RX_FIFO_OVERWRITE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Start the FDCAN module
    if (HAL_FDCAN_Start(&hfdcan) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef CAN_BUS::configSTM32FDCANFilter(int profile)
{
    FDCAN_FilterTypeDef f;
    f.IdType = FDCAN_STANDARD_ID;
    bool ok = true;

    // Frames que NO coincidan con ningún filtro: por defecto se rechazan.
    uint32_t nonMatching = FDCAN_REJECT;

    switch (profile) {
        case 4: // ---------- VCU ----------
            // Recibe estado del inversor 0x401 / 0x441 / 0x481 (grupo 0x401..0x4E1)
            // y estado del BMS 0x00A (SDC).
            // Filtro 0: MÁSCARA del grupo inversor -> acepta si (id & 0x71F) == 0x401.
            f.FilterIndex  = 0;
            f.FilterType   = FDCAN_FILTER_MASK;
            f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
            f.FilterID1    = 0x401;
            f.FilterID2    = 0x71F;
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            // Filtro 1: DUAL (dos IDs exactos). Solo el BMS 0x00A -> ID1 = ID2.
            f.FilterIndex  = 1;
            f.FilterType   = FDCAN_FILTER_DUAL;
            f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
            f.FilterID1    = 0x00A;
            f.FilterID2    = 0x00A;
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            break;

        case 2: // ---------- PDM ----------
            // Grupo del inversor 0x401..0x4E1 y, además, 0x3E1 (993).
            // OJO: la PDM actual (mart-PDM, Refrigeracion_G474) NO usa este
            // perfil: arranca con el nodo 5 y programa con setFilters() sus
            // IDs reales (0x441 temperaturas y 0x300 consigna de la VCU).
            f.FilterIndex  = 0;
            f.FilterType   = FDCAN_FILTER_MASK;
            f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
            f.FilterID1    = 0x401;
            f.FilterID2    = 0x71F;
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            // Filtro 1: DUAL con 0x3E1 (993) -> ID1 = ID2.
            f.FilterIndex  = 1;
            f.FilterType   = FDCAN_FILTER_DUAL;
            f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
            f.FilterID1    = 0x3E1;
            f.FilterID2    = 0x3E1;
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            break;

        case 3: // ---------- BMS ----------
            // No debe recibir ningún paquete: ambos filtros deshabilitados + rechazo global.
            f.FilterType   = FDCAN_FILTER_MASK;   // valor válido aunque esté deshabilitado
            f.FilterID1    = 0x000;
            f.FilterID2    = 0x000;
            f.FilterIndex  = 0;
            f.FilterConfig = FDCAN_FILTER_DISABLE;
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            f.FilterIndex  = 1;
            f.FilterConfig = FDCAN_FILTER_DISABLE;
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            nonMatching = FDCAN_REJECT;
            break;

        default: // ---------- Acepta todo ----------
            f.FilterIndex  = 0;
            f.FilterType   = FDCAN_FILTER_MASK;
            f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
            f.FilterID1    = 0x000;
            f.FilterID2    = 0x000;   // máscara 0 -> acepta todo
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            f.FilterIndex  = 1;
            f.FilterType   = FDCAN_FILTER_MASK;
            f.FilterConfig = FDCAN_FILTER_DISABLE;
            ok &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
            nonMatching = FDCAN_ACCEPT_IN_RX_FIFO0;
            break;
    }

    ok &= (HAL_FDCAN_ConfigGlobalFilter(
        &hfdcan,
        nonMatching,                // Non-matching std frames
        nonMatching,                // Non-matching ext frames
        FDCAN_REJECT_REMOTE,        // Remote std frames -> rechazadas
        FDCAN_REJECT_REMOTE         // Remote ext frames -> rechazadas
    ) == HAL_OK);

    return ok ? HAL_OK : HAL_ERROR;
}

// Programa el filtro HW con los IDs de filterIDs: filtros DUAL, dos IDs exactos
// por elemento. Hay que parar el FDCAN porque el filtro global (que decide que
// pasa con lo que no coincide) solo se puede cambiar en estado READY; la parada
// dura microsegundos y se hace una vez, en el setup.
bool CAN_BUS::applySTM32FDCANFilterIds()
{
    std::vector<uint32_t> stdIds;
    bool anyExtended = false;
    for (unsigned long id : filterIDs)
    {
        if (id <= 0x7FF)
            stdIds.push_back(id);
        else
            anyExtended = true;
    }
    if (stdIds.empty() || (stdIds.size() + 1) / 2 > STM32_FDCAN_STD_FILTERS)
    {
        return false; // se queda el filtro del perfil; el software sigue filtrando
    }

    if (HAL_FDCAN_Stop(&hfdcan) != HAL_OK)
    {
        return false;
    }

    FDCAN_FilterTypeDef f;
    f.IdType = FDCAN_STANDARD_ID;
    bool filtersOk = true;
    uint32_t index = 0;
    for (size_t i = 0; i < stdIds.size(); i += 2)
    {
        f.FilterIndex  = index++;
        f.FilterType   = FDCAN_FILTER_DUAL;
        f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
        f.FilterID1    = stdIds[i];
        f.FilterID2    = (i + 1 < stdIds.size()) ? stdIds[i + 1] : stdIds[i];
        filtersOk &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
    }
    // El resto de elementos (los que hubiera puesto el perfil) se deshabilitan.
    f.FilterType   = FDCAN_FILTER_MASK;
    f.FilterConfig = FDCAN_FILTER_DISABLE;
    f.FilterID1    = 0x000;
    f.FilterID2    = 0x000;
    for (; index < STM32_FDCAN_STD_FILTERS; index++)
    {
        f.FilterIndex = index;
        filtersOk &= (HAL_FDCAN_ConfigFilter(&hfdcan, &f) == HAL_OK);
    }

    // Estandar que no coincide -> fuera. Si algun filtro no se pudo escribir se
    // acepta todo: mejor que filtre el software que perder un ID que si se quiere.
    // Extendidas: si hay alguna en la lista pasan todas y las criba el software.
    bool ok = filtersOk;
    ok &= (HAL_FDCAN_ConfigGlobalFilter(
        &hfdcan,
        filtersOk   ? FDCAN_REJECT : FDCAN_ACCEPT_IN_RX_FIFO0,
        anyExtended ? FDCAN_ACCEPT_IN_RX_FIFO0 : FDCAN_REJECT,
        FDCAN_REJECT_REMOTE,
        FDCAN_REJECT_REMOTE
    ) == HAL_OK);

    // Se arranca SIEMPRE, haya fallado lo que haya fallado: sin Start no hay CAN.
    ok &= (HAL_FDCAN_Start(&hfdcan) == HAL_OK);
    return ok;
}
#endif