//********MART CAN CONVERTER LIBRARY
#ifndef MARTCAN_H
#define MARTCAN_H

#include <Arduino.h>
#include <bitset>
#include <cstring>
#include <optional>
#include "mcp_can.h"
#include "CAN_DATA.h"
#include "common.h"
#include "MCP2515_Config.h"
#include <ESP32-TWAI-CAN.hpp>


    enum class HardwareType
    {
        Controller,
        Transciever
    };
class CAN_BUS
{

public:
    // CONVERTER converter;
    MCP_CAN _CAN;
    CAN_DATA DataIN, DataOUT;

    HardwareType type;
    int error;

    struct Config
    {
        bool respondToRRF;            // Automaticaly respond to a RRF
        bool autoRemoveRRFPacket;     // Automaticaly delete a received rrf when the requested data is sent
        bool simulating;              // Set to true for testing when no MCP2515 are connected to the microcontroller
        bool autoRemoveStoredFilters; // Removes the stored masks and filters when applied to the MCP2515 registers to save memory
        bool sendStatusData;          // Send status data such as runtime time, number of sent, received and collided paquets.
    } config;

    // Constructor: Initializes the MCP_CAN instance and sets up the CAN interface
    
    CAN_BUS(int pinCs) : _CAN(pinCs)
    {
        if (_CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
            Serial.println("MCP2515 Initialized Successfully!");
        else
            Serial.println("Error Initializing MCP2515...");
        _CAN.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted

        // Default configuration
        config.respondToRRF = true;
        config.autoRemoveRRFPacket = true;
        config.simulating = false;
        config.autoRemoveStoredFilters = true;
        config.sendStatusData = false;
    }

    // Constructor: Initializes the MCP_CAN instance and sets up the CAN interface
    CAN_BUS(int pinCs, int _nodeID) : _CAN(pinCs)
    {
        if (_CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK)
            Serial.println("MCP2515 Initialized Successfully!");
        else
            Serial.println("Error Initializing MCP2515...");
        _CAN.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted

        // Default configuration
        config.respondToRRF = true;
        config.autoRemoveRRFPacket = true;
        config.simulating = false;
        config.autoRemoveStoredFilters = true;
        config.sendStatusData = false;

        // unsigned long int rIDS[STATUS_NUM_IDS];
        // for(unsigned long i=0;i<STATUS_NUM_IDS;i++)
        // {
        //     rIDS[i]=STATUS_START_MASTER_ID+i;
        // }
        // DataIN.setRemovableIds(rIDS,STATUS_NUM_IDS);

        // statusPacketOffset=STATUS_START_MASTER_ID+(_nodeID-1)*STATUS_NUM_PAQUETS;
        // for(unsigned i=0;i<STATUS_NUM_PAQUETS;i++)
        // {
        //     //Sets CANStatusPackets timer
        //     setPacketTimer(statusPacketOffset+i,STATUS_DATA_TIME_CALC);

        //     //Store the CANStatusPackets ID's in filterIDs so they can be read by the other ESP's
        //     filterIDs.push_back(statusPacketOffset+i);
        // }
        previousStatusIntervalTime = millis();
        previousStatusRuntimeTime = millis();
        intervalTime = STATUS_DATA_TIME_CALC;
        nodeID = _nodeID;
    }

    // Destructor
    ~CAN_BUS() {}


    // Constructor: Initializes the transciever or controller instance and sets up the CAN interface
    CAN_BUS(HardwareType type, unsigned int speed, int _nodeID, int pinCs = 0) : _CAN(pinCs){

         //**CORREGIR: La configuración de abajo (salvo los config.___) debe de hacerse llamando a un método público de la clase CAN_BUS que tenéis que crear (Por ejemplo setupCANHardware("argumentos de configuración")).
        //** Este constructor llamará a dicho método aquí (pasándole los valores de configuración a traves de los parámetros ), es simplemente mover el código de abajo al cuerpo de el método de configuración.
        //** Motivo: La velocidad del bus debe de poder ser cambiada "en caliente" cuando se conecte el carro al BMS sin llamar al constructor" 
        //** NOTA: la función setupCANHardware debe de tener en cuenta el hardware escogido e implementar la lógica necesaria para configurar uno u otro*/
        error = 0;
        this->type = type;
        if(this->type == HardwareType::Controller){
            //**CORREGIR: Incluir inicialización del MCP2515
        }else if(this->type == HardwareType::Transciever){
            ESP32Can.setSpeed(ESP32Can.convertSpeed(speed));
            //**CORREGIR: Cambiar el "500" por la velocidad genérica, probando en concreto si todo funciona a 1Mbps
            if(!ESP32Can.begin(ESP32Can.convertSpeed(500), 5, 4, 10, 10)){
                Serial.println("Error Initializing ESP32Can...");
                error =1;
        }
        config.respondToRRF = true;
        config.autoRemoveRRFPacket = true;
        config.simulating = false;
        config.autoRemoveStoredFilters = true;
        config.sendStatusData = false;
    }
    }
    // Sends all stored data packets in DataOUT
    bool send();
    //setup for transciever or controller

    int SetupState(){
        return error;
    }

    // Sends a specific stored data packet in DataOUT
    bool send(unsigned long id);

    // Sends a specific stored data packet in DataOUT
    bool sendRequestedRRF(unsigned long id);

    // Receives data packets and stores them in DataIN
    void receive();

    // Retrieves a packet with a specific CAN ID and unpacks its data
    template <typename... Args>
    bool getPacket(unsigned long canId, Args &...args)
    {
        bool ok = true;
        std::size_t size = (arraySizeInBits(args) + ...);
        if (size > 64)
        {
            ERROR_PRINTLN((String) "Error!, packetID: " + canId + " size = " + size + " >64");
            ok = false;
        }

        else
        {
            const CanPacketRawData *packet = DataIN.getPacketById(canId);
            if (packet != nullptr)
            {
                unpackCANMessage(packet->bytes, args...);
                ok = true;
            }
            else
            {
                ERROR_PRINTLN("Error: No matching packet found.");
                ok = false;
            }
        }
        return (ok || config.simulating);
    }

    // Retrieves the last received packet and unpacks its data
    template <typename... Args>
    bool getPacket(Args &...args)
    {
        bool ok = true;
        if (DataIN.lastAddedPacket != nullptr)
        {
            unpackCANMessage(DataIN.lastAddedPacket->bytes, args...);
            ok = true;
        }
        else
        {
            ERROR_PRINTLN("Error: No packet has been added yet.");
            ok = false;
        }
        return (ok || config.simulating);
    }

    // Packs provided data into a CAN packet and stores it in DataOUT
    template <typename... Args>
    bool setPacket(unsigned long canId, Args &&...args)
    {
        bool ok = true;
        std::size_t size = (arraySizeInBits(args) + ...);
        if (size > 64)
        {
            ERROR_PRINTLN((String) "Error!, packetID: " + canId + " size = " + size + " >8");
            ok = false;
        }
        else
        {

            DataOUT.dataRaw.size = 8;
            uint8_t *outputArray = DataOUT.dataRaw.bytes;
            std::fill_n(outputArray, 8, 0x00); // Initialize with 0x00
            size_t offset = 0;
            (..., (offset = packArgument(std::forward<Args>(args), outputArray, offset)));
            DataOUT.dataRaw.id = canId;

            DataOUT.dataRaw.typeExtendedId = DataOUT.dataRaw.id > 0x7FF;
            DataOUT.dataRaw.rrf = false;
            // Checks if there is a RRF rule stored involving that packet. If so, make
            // WaitForRRF true so send() doesn't send that package unless a rrf is received
            if (searchOutId(canId))
            {
                DataOUT.dataRaw.WaitForRRF = true;
            }
            else
            {
                DataOUT.dataRaw.WaitForRRF = false;
            }
            DataOUT.addPacket(DataOUT.dataRaw);
        }
        return ok;
    }

    // Packs RRF message
    void setPacket(unsigned long canId)
    {
        DataOUT.dataRaw.size = 8;
        DataOUT.dataRaw.rrf = true;
        DataOUT.dataRaw.id = canId;
        DataOUT.dataRaw.typeExtendedId = (DataOUT.dataRaw.id > 0x7FF);
        DataOUT.dataRaw.WaitForRRF = false;
        DataOUT.addPacket(DataOUT.dataRaw);
    }

    // Function template to calculate size in bits of a single array
    template <typename T, std::size_t N>
    constexpr std::size_t arraySizeInBits(const T (&)[N])
    {
        return sizeof(T) * N * 8; // Size of each element times number of elements times 8 bits/byte
    }
    // Function template to calculate size in bits of a single array
    template <std::size_t N>
    constexpr std::size_t arraySizeInBits(const bool (&)[N])
    {
        return N; // 1 bit for each bool
    }

    // Helper function to pack a single array

    void printByteArray(const uint8_t *array, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            if (array[i] < 0x10)
                Serial.print("0");
            Serial.print(array[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    // Template function to print an array of any type and size
    template <typename T, size_t N>
    void printArray(const T (&array)[N])
    {
        Serial.print("[");
        for (size_t i = 0; i < N; ++i)
        {
            if (i > 0)
            {
                Serial.print(", ");
            }
            Serial.print(array[i]);
        }
        Serial.println("]");
    }
    // Configures the RRF pairs
    void setRRFId(unsigned long inId, unsigned long outId);

    // Calculates and writes the masks and filters to the MCP2515 registers given a set of IDs
    bool setFilters(const unsigned long ids[], unsigned size);

    // Prints the calculated masks and filters
    void printFilters();

    // Test by software if the given IDs are accepted by the created filters or not
    void testFilters(const std::vector<uint16_t> &testIds);

    void setPacketTimer(unsigned long packetID, unsigned long time);
    void printStatusData(unsigned _nodeID);
    void printReceivedIds();

    //** CAN STATUS DATA**//
    bool getCANStatusData(unsigned _nodeid, int d[]);

    //** CAN BUS STATUS DATA **//
    unsigned nodeID, statusPacketOffset;                                                             // IDs
    unsigned runtimeTime, numRXPaqOK, numTXPaqOK, numTxPaqError;                                     // Actual data
    unsigned previousStatusIntervalTime, previousStatusRuntimeTime, intervalTime, numCurrentSamples; // Aux data

private:
    struct RRFIds
    {
        std::vector<unsigned long> INRRFid;
        std::vector<unsigned long> OUTRRFid;
    };
    struct PacketTimer
    {
        unsigned long packetID;
        unsigned long interval;
    };
    std::vector<PacketTimer> packetTimers;

    std::vector<RRFIds> rrfIdsList;       // Vector holding INRRFid and OUTRRFid vectors
    std::vector<unsigned long> filterIDs; // Vector holding INRRFid and OUTRRFid vectors
    MCP2515Configurator configurator;

    bool readBytes();
    bool writeBytes();
    // Method to search for an InID and return the associated OUTids vector
    std::optional<std::vector<unsigned long>> getOutIdsByInId(unsigned long inId);

    // Method to search for an OUTid and return true if found
    bool searchOutId(unsigned long outId);

    //Change the speed of the bus while running (ms)
    void ChangeSpeed(unsigned int speed){
        ESP32Can.setSpeed(ESP32Can.convertSpeed(speed));
    }

    // Method to print all RRFIds
    void printRRFIds()
    {
        for (const auto &rrfIds : rrfIdsList)
        {
            Serial.print("INRRFid: ");
            for (const auto &id : rrfIds.INRRFid)
            {
                Serial.print(id);
                Serial.print(" ");
            }
            Serial.println(); // New line after printing INRRFid

            Serial.print("OUTRRFid: ");
            for (const auto &id : rrfIds.OUTRRFid)
            {
                Serial.print(id);
                Serial.print(" ");
            }
            Serial.println(); // New line after printing OUTRRFid
        }
    }

    template <typename T, size_t N>
    size_t packSingleArray(const T (&array)[N], uint8_t *outputArray, size_t offset)
    {
        // static_assert(N * sizeof(T) + offset <= 8, "Data exceeds CAN message size limit.");

        for (size_t i = 0; i < N; ++i)
        {
            const uint8_t *elementBytes = reinterpret_cast<const uint8_t *>(&array[i]);
            size_t elementSize = sizeof(T);
            for (size_t byteIndex = 0; byteIndex < elementSize; ++byteIndex)
            {
                if (offset < 8)
                {
                    outputArray[offset++] = elementBytes[elementSize - 1 - byteIndex]; // Reverse the byte order
                }
            }
        }
        return offset;
    }

    size_t packBoolArrayAsBits(const bool *array, size_t N, uint8_t *outputArray, size_t offset)
    {
        size_t byteIndex = 0;    // Index of the current byte in the output array
        uint8_t currentByte = 0; // Current byte being constructed
        size_t bitIndex = 0;     // Bit position in the current byte

        for (size_t i = 0; i < N; ++i)
        {
            // Set the corresponding bit in currentByte if the boolean value is true
            if (array[i])
            {
                currentByte |= (1 << bitIndex);
            }

            // Move to the next bit
            bitIndex++;

            // Check if we have filled up the current byte or reached the end of the array
            if (bitIndex == 8 || i == N - 1)
            {
                // Store the constructed byte in the output array
                if (offset + byteIndex < 8)
                { // Check to avoid writing beyond the output array
                    outputArray[offset + byteIndex] = currentByte;
                }
                byteIndex++;
                bitIndex = 0;
                currentByte = 0; // Reset for the next byte
            }
        }

        return offset + byteIndex; // Return the new offset
    }
    // Helper function to process a single argument
    template <typename T, size_t N>
    size_t processArgument(const T (&array)[N], uint8_t *outputArray, size_t offset)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            // Special handling for boolean arrays
            return packBoolArrayAsBits(array, N, outputArray, offset);
        }
        else
        {
            // Default handling for other types
            return packSingleArray(array, outputArray, offset);
        }
    }

    template <typename T>
    size_t packArgument(const T &arg, uint8_t *outputArray, size_t offset)
    {
        if constexpr (std::is_same_v<T, float>)
        {
            // Special handling for float
            const uint8_t *elementBytes = reinterpret_cast<const uint8_t *>(&arg);
            for (size_t byteIndex = 0; byteIndex < sizeof(T); ++byteIndex)
            {
                if (offset < 8)
                {
                    outputArray[offset++] = elementBytes[sizeof(T) - 1 - byteIndex]; // Reverse the byte order for big-endian
                }
            }
        }
        else
        {
            // Default handling for other types
            return processArgument(arg, outputArray, offset);
        }
        return offset;
    }

    // Specialization for array types
    template <typename T, size_t N>
    size_t packArgument(const T (&array)[N], uint8_t *outputArray, size_t offset)
    {
        return processArgument(array, outputArray, offset); // Directly call processArgument
    }

    ///*****************************************UNPACK*****************************************

    template <size_t N>
    void unpackArray(const uint8_t *&inputArray, int (&outputArray)[N], size_t &offset)
    {
        for (size_t i = 0; i < N; ++i)
        {
            outputArray[i] = 0;
            // Convert from big-endian to little-endian
            for (int byte = sizeof(int) - 1; byte >= 0; --byte)
            {
                outputArray[i] |= static_cast<int>(inputArray[offset + byte]) << ((sizeof(int) - 1 - byte) * 8);
            }
            offset += sizeof(int);
        }
    }
    template <size_t N>
    void unpackArray(const uint8_t *&inputArray, unsigned long (&outputArray)[N], size_t &offset)
    {
        for (size_t i = 0; i < N; ++i)
        {
            outputArray[i] = 0;
            // Convert from big-endian to little-endian
            for (int byte = sizeof(unsigned long) - 1; byte >= 0; --byte)
            {
                outputArray[i] |= static_cast<unsigned long>(inputArray[offset + byte]) << ((sizeof(unsigned long) - 1 - byte) * 8);
            }
            offset += sizeof(unsigned long);
        }
    }

    template <size_t N>
    void unpackArray(const uint8_t *&inputArray, short (&outputArray)[N], size_t &offset)
    {
        for (size_t i = 0; i < N; ++i)
        {
            outputArray[i] = 0;
            // Convert from big-endian to little-endian
            for (int byte = sizeof(short) - 1; byte >= 0; --byte)
            {
                outputArray[i] |= static_cast<short>(inputArray[offset + byte]) << ((sizeof(short) - 1 - byte) * 8);
            }
            offset += sizeof(short);
        }
    }

    template <size_t N>
    void unpackArray(const uint8_t *&inputArray, bool (&outputArray)[N], size_t &offset)
    {
        for (size_t i = 0; i < N; ++i)
        {
            // Extract each bit as a boolean value
            size_t byteIndex = offset;
            size_t bitIndex = (i % 8); // Adjust for big-endian bit order
            outputArray[i] = (inputArray[byteIndex] >> bitIndex) & 0x01;

            if (i % 8 == 7)
            {
                offset += 1; // Move to the next byte after 8 bits
            }
        }
    }
    template <size_t N>
    void unpackArray(const uint8_t *&inputArray, float (&outputArray)[N], size_t &offset)
    {
        for (size_t i = 0; i < N; ++i)
        {
            uint8_t elementBytes[sizeof(float)];
            // Reassemble the bytes into a float
            for (int byte = sizeof(float) - 1; byte >= 0; --byte)
            {
                elementBytes[sizeof(float) - 1 - byte] = inputArray[offset + byte]; // Reverse for big-endian
            }
            float value;
            std::memcpy(&value, elementBytes, sizeof(float));
            outputArray[i] = value;
            offset += sizeof(float);
        }
    }

    template <size_t N>
    void unpackArray(const uint8_t *&inputArray, uint8_t (&outputArray)[N], size_t &offset)
    {
        for (size_t i = 0; i < N; ++i)
        {
            // Directly assign each byte from inputArray to outputArray
            outputArray[i] = inputArray[offset];
            offset += 1;
        }
    }

    template <typename T, typename... Args>
    void unpackCANMessage(const uint8_t *inputArray, size_t &offset, T &first, Args &...rest)
    {
        unpackArray(inputArray, first, offset);
        if constexpr (sizeof...(rest) > 0)
        {
            unpackCANMessage(inputArray, offset, rest...);
        }
    }

    // Overload for starting the recursion
    template <typename... Args>
    void unpackCANMessage(const uint8_t *inputArray, Args &...args)
    {
        size_t offset = 0;
        unpackCANMessage(inputArray, offset, args...);
    }

    //** CAN STATUS **//
    void setCANStatusData();
    void getCANStatusData(unsigned _nodeid, int d0[], int d1[], int d2[], bool &ok);
};

#endif