#include <iostream>
#include <vector>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <stdexcept>

// Define byte type if not already defined
using byte = unsigned char;

// General template for setPacket (works for integer types)
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

// Specialization for float type
template<>
void setPacket<float>(const float* data, size_t dataSize, byte outputArray[]) {
    // Calculate total size needed
    size_t dataBytes = sizeof(float) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: data exceeds 8-byte limit");
    }
    
    // Clear the output array
    std::memset(outputArray, 0, 8);
    
    // Convert from little endian to big endian and copy
    if (data != nullptr && dataSize > 0) {
        for (size_t i = 0; i < dataSize; ++i) {
            // Use a union to access the bytes of the float
            union {
                float f;
                uint32_t i;
            } converter;
            
            converter.f = data[i];
            uint32_t value = converter.i;
            size_t elementSize = sizeof(float);
            
            // Calculate starting position for this element in the output array
            size_t startPos = i * elementSize;
            
            // Copy bytes in reverse order (little endian to big endian)
            for (size_t j = 0; j < elementSize; ++j) {
                outputArray[startPos + j] = static_cast<byte>((value >> ((elementSize - 1 - j) * 8)) & 0xFF);
            }
        }
    }
}

// Specialization for double type
template<>
void setPacket<double>(const double* data, size_t dataSize, byte outputArray[]) {
    // Calculate total size needed
    size_t dataBytes = sizeof(double) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: data exceeds 8-byte limit");
    }
    
    // Clear the output array
    std::memset(outputArray, 0, 8);
    
    // Convert from little endian to big endian and copy
    if (data != nullptr && dataSize > 0) {
        for (size_t i = 0; i < dataSize; ++i) {
            // Use a union to access the bytes of the double
            union {
                double d;
                uint64_t i;
            } converter;
            
            converter.d = data[i];
            uint64_t value = converter.i;
            size_t elementSize = sizeof(double);
            
            // Calculate starting position for this element in the output array
            size_t startPos = i * elementSize;
            
            // Copy bytes in reverse order (little endian to big endian)
            for (size_t j = 0; j < elementSize; ++j) {
                outputArray[startPos + j] = static_cast<byte>((value >> ((elementSize - 1 - j) * 8)) & 0xFF);
            }
        }
    }
}

// General template for getPacket (works for integer types)
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

// Specialization for float type
template<>
void getPacket<float>(const byte inputArray[], float* data, size_t dataSize) {
    // Calculate size
    size_t dataBytes = sizeof(float) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: trying to extract more data than available");
    }
    
    // Extract the data and convert from big endian to little endian
    if (data != nullptr && dataSize > 0) {
        for (size_t i = 0; i < dataSize; ++i) {
            uint32_t value = 0;
            size_t elementSize = sizeof(float);
            
            // Calculate starting position for this element in the input array
            size_t startPos = i * elementSize;
            
            // Convert big endian to little endian by reading bytes in reverse
            for (size_t j = 0; j < elementSize; ++j) {
                // Shift existing value and add next byte
                value = (value << 8) | inputArray[startPos + j];
            }
            
            // Use a union to convert uint32_t back to float
            union {
                uint32_t i;
                float f;
            } converter;
            
            converter.i = value;
            data[i] = converter.f;
        }
    }
}

// Specialization for double type
template<>
void getPacket<double>(const byte inputArray[], double* data, size_t dataSize) {
    // Calculate size
    size_t dataBytes = sizeof(double) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: trying to extract more data than available");
    }
    
    // Extract the data and convert from big endian to little endian
    if (data != nullptr && dataSize > 0) {
        for (size_t i = 0; i < dataSize; ++i) {
            uint64_t value = 0;
            size_t elementSize = sizeof(double);
            
            // Calculate starting position for this element in the input array
            size_t startPos = i * elementSize;
            
            // Convert big endian to little endian by reading bytes in reverse
            for (size_t j = 0; j < elementSize; ++j) {
                // Shift existing value and add next byte
                value = (value << 8) | inputArray[startPos + j];
            }
            
            // Use a union to convert uint64_t back to double
            union {
                uint64_t i;
                double d;
            } converter;
            
            converter.i = value;
            data[i] = converter.f;
        }
    }
}

// Helper function to print a byte array
void printByteArray(const byte array[], size_t size) {
    std::cout << "[ ";
    for (size_t i = 0; i < size; ++i) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(array[i]);
        if (i < size - 1) {
            std::cout << ", ";
        }
    }
    std::cout << std::dec << " ]" << std::endl;
}

// Test function for uint8_t (byte) data
void testUint8() {
    std::cout << "Testing uint8_t data..." << std::endl;
    
    // Test single value
    {
        uint8_t input[] = {0xAB};
        byte output[8] = {0};
        
        setPacket(input, 1, output);
        std::cout << "Input: 0x" << std::hex << static_cast<int>(input[0]) << std::dec << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        uint8_t result[1] = {0};
        getPacket(output, result, 1);
        std::cout << "Result: 0x" << std::hex << static_cast<int>(result[0]) << std::dec << std::endl;
        
        assert(result[0] == input[0]);
        std::cout << "Single uint8_t test passed!" << std::endl;
    }
    
    // Test multiple values
    {
        uint8_t input[] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
        byte output[8] = {0};
        
        setPacket(input, 8, output);
        std::cout << "\nInput multiple uint8_t: ";
        for (int i = 0; i < 8; i++) {
            std::cout << "0x" << std::hex << static_cast<int>(input[i]);
            if (i < 7) std::cout << ", ";
        }
        std::cout << std::dec << std::endl;
        
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        uint8_t result[8] = {0};
        getPacket(output, result, 8);
        std::cout << "Result: ";
        for (int i = 0; i < 8; i++) {
            std::cout << "0x" << std::hex << static_cast<int>(result[i]);
            if (i < 7) std::cout << ", ";
        }
        std::cout << std::dec << std::endl;
        
        for (int i = 0; i < 8; i++) {
            assert(result[i] == input[i]);
        }
        std::cout << "Multiple uint8_t test passed!" << std::endl;
    }
    
    std::cout << "All uint8_t tests passed!\n" << std::endl;
}

// Test function for uint16_t data
void testUint16() {
    std::cout << "Testing uint16_t data..." << std::endl;
    
    // Test single value
    {
        uint16_t input[] = {0xABCD};
        byte output[8] = {0};
        
        setPacket(input, 1, output);
        std::cout << "Input: 0x" << std::hex << input[0] << std::dec << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        uint16_t result[1] = {0};
        getPacket(output, result, 1);
        std::cout << "Result: 0x" << std::hex << result[0] << std::dec << std::endl;
        
        assert(result[0] == input[0]);
        std::cout << "Single uint16_t test passed!" << std::endl;
    }
    
    // Test multiple values
    {
        uint16_t input[] = {0x1234, 0x5678, 0x9ABC, 0xDEF0};
        byte output[8] = {0};
        
        setPacket(input, 4, output);
        std::cout << "\nInput multiple uint16_t: ";
        for (int i = 0; i < 4; i++) {
            std::cout << "0x" << std::hex << input[i];
            if (i < 3) std::cout << ", ";
        }
        std::cout << std::dec << std::endl;
        
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        uint16_t result[4] = {0};
        getPacket(output, result, 4);
        std::cout << "Result: ";
        for (int i = 0; i < 4; i++) {
            std::cout << "0x" << std::hex << result[i];
            if (i < 3) std::cout << ", ";
        }
        std::cout << std::dec << std::endl;
        
        for (int i = 0; i < 4; i++) {
            assert(result[i] == input[i]);
        }
        std::cout << "Multiple uint16_t test passed!" << std::endl;
    }
    
    std::cout << "All uint16_t tests passed!\n" << std::endl;
}

// Test function for uint32_t data
void testUint32() {
    std::cout << "Testing uint32_t data..." << std::endl;
    
    // Test single value
    {
        uint32_t input[] = {0x12345678};
        byte output[8] = {0};
        
        setPacket(input, 1, output);
        std::cout << "Input: 0x" << std::hex << input[0] << std::dec << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        uint32_t result[1] = {0};
        getPacket(output, result, 1);
        std::cout << "Result: 0x" << std::hex << result[0] << std::dec << std::endl;
        
        assert(result[0] == input[0]);
        std::cout << "Single uint32_t test passed!" << std::endl;
    }
    
    // Test multiple values
    {
        uint32_t input[] = {0x12345678, 0x9ABCDEF0};
        byte output[8] = {0};
        
        setPacket(input, 2, output);
        std::cout << "\nInput multiple uint32_t: ";
        for (int i = 0; i < 2; i++) {
            std::cout << "0x" << std::hex << input[i];
            if (i < 1) std::cout << ", ";
        }
        std::cout << std::dec << std::endl;
        
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        uint32_t result[2] = {0};
        getPacket(output, result, 2);
        std::cout << "Result: ";
        for (int i = 0; i < 2; i++) {
            std::cout << "0x" << std::hex << result[i];
            if (i < 1) std::cout << ", ";
        }
        std::cout << std::dec << std::endl;
        
        for (int i = 0; i < 2; i++) {
            assert(result[i] == input[i]);
        }
        std::cout << "Multiple uint32_t test passed!" << std::endl;
    }
    
    std::cout << "All uint32_t tests passed!\n" << std::endl;
}

// Test function for uint64_t data
void testUint64() {
    std::cout << "Testing uint64_t data..." << std::endl;
    
    // Test single value
    {
        uint64_t input[] = {0x123456789ABCDEF0};
        byte output[8] = {0};
        
        setPacket(input, 1, output);
        std::cout << "Input: 0x" << std::hex << input[0] << std::dec << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        uint64_t result[1] = {0};
        getPacket(output, result, 1);
        std::cout << "Result: 0x" << std::hex << result[0] << std::dec << std::endl;
        
        assert(result[0] == input[0]);
        std::cout << "Single uint64_t test passed!" << std::endl;
    }
    
    std::cout << "All uint64_t tests passed!\n" << std::endl;
}

// Test for error cases
void testErrorCases() {
    std::cout << "Testing error cases..." << std::endl;
    
    // Test overflow with setPacket
    {
        uint32_t input[] = {0x12345678, 0x9ABCDEF0, 0x11223344};  // 12 bytes total (exceeds 8-byte limit)
        byte output[8] = {0};
        
        bool exceptionCaught = false;
        try {
            setPacket(input, 3, output);
        } catch (const std::overflow_error& e) {
            exceptionCaught = true;
            std::cout << "Expected exception caught: " << e.what() << std::endl;
        }
        
        assert(exceptionCaught);
        std::cout << "setPacket overflow test passed!" << std::endl;
    }
    
    // Test overflow with getPacket
    {
        byte input[8] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
        uint32_t output[3] = {0};  // 12 bytes total (exceeds 8-byte limit)
        
        bool exceptionCaught = false;
        try {
            getPacket(input, output, 3);
        } catch (const std::overflow_error& e) {
            exceptionCaught = true;
            std::cout << "Expected exception caught: " << e.what() << std::endl;
        }
        
        assert(exceptionCaught);
        std::cout << "getPacket overflow test passed!" << std::endl;
    }
    
    // Test with null pointers
    {
        byte output[8] = {0};
        
        // This should not crash, just do nothing
        setPacket<uint32_t>(nullptr, 2, output);
        
        uint32_t result[2] = {0};
        // This should not crash, just do nothing
        getPacket<uint32_t>(output, nullptr, 2);
        
        std::cout << "Null pointer tests passed!" << std::endl;
    }
    
    // Test with zero size
    {
        uint32_t input[] = {0x12345678, 0x9ABCDEF0};
        byte output[8] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};  // Fill with pattern
        
        // This should just clear the output array
        setPacket(input, 0, output);
        
        // Check that the output array was cleared
        for (int i = 0; i < 8; i++) {
            assert(output[i] == 0);
        }
        
        std::cout << "Zero size test passed!" << std::endl;
    }
    
    std::cout << "All error tests passed!\n" << std::endl;
}

// Test mixed data types
void testMixedTypes() {
    std::cout << "Testing mixed data types..." << std::endl;
    
    // Test with int8_t (signed)
    {
        int8_t input[] = {-1, -2, 127, -128};  // Test boundary values
        byte output[8] = {0};
        
        setPacket(input, 4, output);
        std::cout << "Input int8_t: " << static_cast<int>(input[0]) << ", " 
                 << static_cast<int>(input[1]) << ", " 
                 << static_cast<int>(input[2]) << ", " 
                 << static_cast<int>(input[3]) << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        int8_t result[4] = {0};
        getPacket(output, result, 4);
        std::cout << "Result: " << static_cast<int>(result[0]) << ", " 
                 << static_cast<int>(result[1]) << ", " 
                 << static_cast<int>(result[2]) << ", " 
                 << static_cast<int>(result[3]) << std::endl;
        
        for (int i = 0; i < 4; i++) {
            assert(result[i] == input[i]);
        }
        std::cout << "int8_t test passed!" << std::endl;
    }
    
    // Test with int16_t (signed)
    {
        int16_t input[] = {-1, 32767, -32768};
        byte output[8] = {0};
        
        setPacket(input, 3, output);
        std::cout << "\nInput int16_t: " << input[0] << ", " << input[1] << ", " << input[2] << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        int16_t result[3] = {0};
        getPacket(output, result, 3);
        std::cout << "Result: " << result[0] << ", " << result[1] << ", " << result[2] << std::endl;
        
        for (int i = 0; i < 3; i++) {
            assert(result[i] == input[i]);
        }
        std::cout << "int16_t test passed!" << std::endl;
    }
    
    // Test with float
    {
        float input[] = {3.14159f, -2.71828f};
        byte output[8] = {0};
        
        setPacket(input, 2, output);
        std::cout << "\nInput float: " << input[0] << ", " << input[1] << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        float result[2] = {0};
        getPacket(output, result, 2);
        std::cout << "Result: " << result[0] << ", " << result[1] << std::endl;
        
        // Use memcmp for bit-exact comparison of floating point
        assert(memcmp(input, result, sizeof(float) * 2) == 0);
        std::cout << "float test passed!" << std::endl;
    }
    
    // Test with double (one value since it's 8 bytes)
    {
        double input[] = {3.14159265358979};
        byte output[8] = {0};
        
        setPacket(input, 1, output);
        std::cout << "\nInput double: " << input[0] << std::endl;
        std::cout << "Output: ";
        printByteArray(output, 8);
        
        double result[1] = {0};
        getPacket(output, result, 1);
        std::cout << "Result: " << result[0] << std::endl;
        
        // Use memcmp for bit-exact comparison of floating point
        assert(memcmp(input, result, sizeof(double)) == 0);
        std::cout << "double test passed!" << std::endl;
    }
    
    std::cout << "All mixed type tests passed!\n" << std::endl;
}

// Helper function to print the 8-byte packet in hexadecimal format.
void printPacket(const byte packet[8]) {
    for (size_t i = 0; i < 8; ++i) {
        // Print each byte in hexadecimal with two digits.
        std::cout << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(packet[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

int main() {
    // Pack and display the packet for the first group: {1, 2, 3, 4}
    int16_t group1[4] = { 1, 2, 3, 4 };
    byte packet1[8];
    try {
        setPacket(group1, 4, packet1);
        std::cout << "Packet for {1, 2, 3, 4}: " << std::endl;
        printPacket(packet1);
    } catch (const std::exception &e) {
        std::cerr << "Error packing group1: " << e.what() << std::endl;
    }
    
    // Pack and display the packet for the second group: {10, 12, 256, 512}
    int16_t group2[4] = { 10, 12, 256, 512 };
    byte packet2[8];
    try {
        setPacket(group2, 4, packet2);
        std::cout << "Packet for {10, 12, 256, 512}: " << std::endl;
        printPacket(packet2);
    } catch (const std::exception &e) {
        std::cerr << "Error packing group2: " << e.what() << std::endl;
    }
    
    return 0;
}



// int main() {
//     std::cout << "Starting packet functions testbench...\n" << std::endl;
    
//     testUint8();
//     testUint16();
//     testUint32();
//     testUint64();
//     testErrorCases();
//     testMixedTypes();
    
//     std::cout << "All tests completed successfully!" << std::endl;
//     return 0;
// }