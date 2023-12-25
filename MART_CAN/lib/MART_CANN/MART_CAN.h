//********MART CAN CONVERTER LIBRARY
#ifndef MARTCAN_H
#define MARTCAN_H

#include <Arduino.h>
#include <bitset>
#include <cstring>
#include "mcp_can.h"
#include "CAN_DATA.h"
#include "common.h"


class CAN_BUS
{

public:
    // CONVERTER converter;
    MCP_CAN _CAN;
    CAN_DATA DataIN, DataOUT;

    // Constructor: Initializes the MCP_CAN instance and sets up the CAN interface
    CAN_BUS(int pinCs) : _CAN(pinCs)
    {
        if (_CAN.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
            Serial.println("MCP2515 Initialized Successfully!");
        else
            Serial.println("Error Initializing MCP2515...");
        _CAN.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted
    }

    // Destructor
    ~CAN_BUS() {}

    // Sends all stored data packets in DataOUT
    bool send();

    // Receives data packets and stores them in DataIN
    void receive();

    // Retrieves a packet with a specific CAN ID and unpacks its data
    template <typename... Args>
    bool getPacket(unsigned long canId, Args &...args)
    {
        const CanPacketRawData *packet = DataIN.getPacketById(canId);
        if (packet != nullptr)
        {
            unpackCANMessage(packet->bytes, args...);
            return true;
        }
        else
        {
            Serial.println("No matching packet found.");
            return false;
        }
    }

    // Retrieves the last received packet and unpacks its data
    template <typename... Args>
    bool getPacket(Args &...args)
    {
        if (DataIN.lastAddedPacket != nullptr)
        {
            unpackCANMessage(DataIN.lastAddedPacket->bytes, args...);
            return true;
        }
        else
        {
            Serial.println("No packet has been added yet.");
            return false;
        }
    }

    // Packs provided data into a CAN packet and stores it in DataOUT
    template <typename... Args>
    void setPacket(unsigned long canId, Args &&...args)
    {
        uint8_t *outputArray = DataOUT.dataRaw.bytes;
        std::fill_n(outputArray, 8, 0x00); // Initialize with 0x00
        size_t offset = 0;
        (..., (offset = packArgument(std::forward<Args>(args), outputArray, offset)));
        DataOUT.dataRaw.id = canId;
        DataOUT.dataRaw.size = sizeof...(Args);
        DataOUT.dataRaw.typeExtendedId = (DataOUT.dataRaw.id & 0x80000000) != 0;
        DataOUT.dataRaw.rrf = (DataOUT.dataRaw.id & 0x40000000) != 0;
        DataOUT.addPacket(DataOUT.dataRaw);
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

private:
    bool readBytes();
    bool writeBytes();

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
};

#endif