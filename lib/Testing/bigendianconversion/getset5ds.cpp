#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <cassert>

using byte = uint8_t;

// Include the setPacket and getPacket implementations here


// Include the implementation of setPacket and getPacket
template<typename T>
void setPacket(const T* data, size_t dataSize, byte outputArray[]) {
    // Calculate total size needed
    size_t dataBytes = sizeof(T) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: data exceeds 8-byte limit");
    }
    
    // Clear the output array
    std::memset(outputArray, 0, 8);
    
    // Convert from little endian to big endian and copy
    if (data != nullptr && dataSize > 0) {
        for (size_t i = 0; i < dataSize; ++i) {
            T value = data[i];
            size_t elementSize = sizeof(T);
            
            // Calculate starting position for this element in the output array
            size_t startPos = i * elementSize;
            
            // Copy bytes in reverse order (little endian to big endian)
            for (size_t j = 0; j < elementSize; ++j) {
                outputArray[startPos + j] = static_cast<byte>((value >> ((elementSize - 1 - j) * 8)) & 0xFF);
            }
        }
    }
}

template<typename T>
void getPacket(const byte inputArray[], T* data, size_t dataSize) {
    // Calculate size
    size_t dataBytes = sizeof(T) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: trying to extract more data than available");
    }
    
    // Extract the data and convert from big endian to little endian
    if (data != nullptr && dataSize > 0) {
        for (size_t i = 0; i < dataSize; ++i) {
            T value = 0;
            size_t elementSize = sizeof(T);
            
            // Calculate starting position for this element in the input array
            size_t startPos = i * elementSize;
            
            // Convert big endian to little endian by reading bytes in reverse
            for (size_t j = 0; j < elementSize; ++j) {
                // Shift existing value and add next byte
                value = (value << 8) | inputArray[startPos + j];
            }
            
            data[i] = value;
        }
    }
}


void testSetGetIntegralTypes() {
    // Test uint8_t (single element)
    {
        byte output[8] = {0xFF};
        uint8_t data = 0xAB;
        setPacket(&data, 1, output);
        assert(output[0] == 0xAB);
        for (size_t i = 1; i < 8; ++i) {
            assert(output[i] == 0);
        }
        uint8_t retrieved;
        getPacket(output, &retrieved, 1);
        assert(retrieved == data);
    }

    // Test uint8_t (multiple elements)
    {
        byte output[8] = {0xFF};
        uint8_t data[] = {0x01, 0x02, 0x03};
        setPacket(data, 3, output);
        assert(memcmp(output, data, 3) == 0);
        for (size_t i = 3; i < 8; ++i) {
            assert(output[i] == 0);
        }
        uint8_t retrieved[3];
        getPacket(output, retrieved, 3);
        assert(memcmp(data, retrieved, 3) == 0);
    }

    // Test uint16_t
    {
        byte output[8] = {0xFF};
        uint16_t data[] = {0x1234, 0x5678};
        setPacket(data, 2, output);
        assert(output[0] == 0x12 && output[1] == 0x34);
        assert(output[2] == 0x56 && output[3] == 0x78);
        for (size_t i = 4; i < 8; ++i) {
            assert(output[i] == 0);
        }
        uint16_t retrieved[2];
        getPacket(output, retrieved, 2);
        assert(retrieved[0] == 0x1234 && retrieved[1] == 0x5678);
    }

    // Test uint32_t
    {
        byte output[8] = {0xFF};
        uint32_t data = 0x12345678;
        setPacket(&data, 1, output);
        const byte expected[] = {0x12, 0x34, 0x56, 0x78};
        assert(memcmp(output, expected, 4) == 0);
        for (size_t i = 4; i < 8; ++i) {
            assert(output[i] == 0);
        }
        uint32_t retrieved;
        getPacket(output, &retrieved, 1);
        assert(retrieved == 0x12345678);
    }

    // Test uint64_t (exact 8 bytes)
    {
        byte output[8] = {0xFF};
        uint64_t data = 0x123456789ABCDEF0;
        setPacket(&data, 1, output);
        const byte expected[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
        assert(memcmp(output, expected, 8) == 0);
        uint64_t retrieved;
        getPacket(output, &retrieved, 1);
        assert(retrieved == data);
    }
}

void testOverflow() {
    byte output[8];

    // setPacket overflow
    {
        uint16_t data[5] = {0}; // 5*2=10 bytes
        bool overflowCaught = false;
        try {
            setPacket(data, 5, output);
        } catch (const std::overflow_error&) {
            overflowCaught = true;
        }
        assert(overflowCaught);
    }

    // getPacket overflow
    {
        uint32_t data[3] = {0}; // 3*4=12 bytes
        byte input[8] = {0};
        bool overflowCaught = false;
        try {
            getPacket(input, data, 3);
        } catch (const std::overflow_error&) {
            overflowCaught = true;
        }
        assert(overflowCaught);
    }
}

void testNullData() {
    // setPacket with nullptr and dataSize=0
    {
        byte output[8] = {0xFF};
        setPacket<uint8_t>(nullptr, 0, output);
        for (byte b : output) {
            assert(b == 0);
        }
    }

    // getPacket with nullptr and dataSize=0 (no crash)
    {
        byte input[8] = {0};
        getPacket<uint8_t>(input, nullptr, 0);
    }
}

void testPartialFill() {
    // Verify remaining bytes are zeroed
    byte output[8] = {0xFF};
    uint8_t data[3] = {0x11, 0x22, 0x33};
    setPacket(data, 3, output);
    assert(memcmp(output, data, 3) == 0);
    for (size_t i = 3; i < 8; ++i) {
        assert(output[i] == 0);
    }
}

int main() {
    testSetGetIntegralTypes();
    testOverflow();
    testNullData();
    testPartialFill();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}