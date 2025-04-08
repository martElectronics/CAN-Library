#include <iostream>
#include <iomanip>
#include <cstring>
#include <stdexcept>
#include <cassert>

// Define "byte" type if not defined
using byte = unsigned char;

// Provided template functions

template<typename T>
void setPacket(const T* data, size_t dataSize, byte outputArray[]) {
    // Calculate total size needed in bytes
    size_t dataBytes = sizeof(T) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: data exceeds 8-byte limit");
    }
    
    // Clear the output array (initialize to zeros)
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
            
            // Convert big endian to little endian by reading bytes in reverse order
            for (size_t j = 0; j < elementSize; ++j) {
                value = (value << 8) | inputArray[startPos + j];
            }
            
            data[i] = value;
        }
    }
}

// Helper function: print the contents of a packet (8-byte array) in hexadecimal.
void printPacket(const byte packet[8]) {
    for (size_t i = 0; i < 8; ++i) {
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
//     // ---------------------------------------------------------
//     // Test 1: Round-trip test with uint16_t using 2 elements.
//     // Expected conversion: little endian to big endian.
//     {
//         std::cout << "Test 1: Round-trip test with uint16_t" << std::endl;
//         uint16_t inputData[2] = {0x1234, 0xABCD};
//         byte packet[8];
        
//         // Pack the data
//         setPacket(inputData, 2, packet);
//         std::cout << "Packet bytes: ";
//         printPacket(packet);
        
//         // Prepare array for unpacked data
//         uint16_t outputData[2] = {0};
        
//         // Unpack the data
//         getPacket(packet, outputData, 2);
        
//         // Check that the input equals the output.
//         assert(inputData[0] == outputData[0]);
//         assert(inputData[1] == outputData[1]);
//         std::cout << "Round-trip uint16_t test passed.\n" << std::endl;
//     }
    
//     // ---------------------------------------------------------
//     // Test 2: Single element test with uint32_t.
//     // Expected conversion: Verify big-endian order in the packet.
//     {
//         std::cout << "Test 2: Single element test with uint32_t" << std::endl;
//         uint32_t inputData[1] = {0xDEADBEEF};
//         byte packet[8];
        
//         // Pack the data
//         setPacket(inputData, 1, packet);
//         std::cout << "Packet bytes: ";
//         printPacket(packet);
        
//         // For a 32-bit value, the first 4 bytes should be the big endian representation of 0xDEADBEEF.
//         // Expected packet bytes: DE AD BE EF 00 00 00 00
//         assert(packet[0] == 0xDE);
//         assert(packet[1] == 0xAD);
//         assert(packet[2] == 0xBE);
//         assert(packet[3] == 0xEF);
//         // Remaining bytes should be zero.
//         for (size_t i = 4; i < 8; ++i) {
//             assert(packet[i] == 0);
//         }
//         // Unpack the data and verify
//         uint32_t outputData[1] = {0};
//         getPacket(packet, outputData, 1);
//         assert(inputData[0] == outputData[0]);
//         std::cout << "Single uint32_t test passed.\n" << std::endl;
//     }
    
//     // ---------------------------------------------------------
//     // Test 3: Exact boundary test with 8 bytes.
//     // For example, using 2 uint32_t (2*4 bytes = 8 bytes).
//     {
//         std::cout << "Test 3: Boundary test with 2 uint32_t (exactly 8 bytes)" << std::endl;
//         uint32_t inputData[2] = {0x11223344, 0xAABBCCDD};
//         byte packet[8];
        
//         // Pack the data, should work since exactly 8 bytes are used.
//         setPacket(inputData, 2, packet);
//         std::cout << "Packet bytes: ";
//         printPacket(packet);
        
//         uint32_t outputData[2] = {0};
//         getPacket(packet, outputData, 2);
//         assert(inputData[0] == outputData[0]);
//         assert(inputData[1] == outputData[1]);
//         std::cout << "Boundary test passed.\n" << std::endl;
//     }
    
//     // ---------------------------------------------------------
//     // Test 4: Overflow test.
//     // For example, attempt to pack 5 uint16_t (5*2=10 bytes) should throw an overflow error.
//     {
//         std::cout << "Test 4: Overflow test with 5 uint16_t (exceeds 8 bytes)" << std::endl;
//         uint16_t inputData[5] = {1, 2, 3, 4, 5};
//         byte packet[8];
//         try {
//             setPacket(inputData, 5, packet);
//             // If no exception, fail the test.
//             std::cerr << "Error: Expected overflow error, but none was thrown." << std::endl;
//             assert(false);
//         } catch (const std::overflow_error &e) {
//             std::cout << "Caught expected exception: " << e.what() << std::endl;
//         }
//         std::cout << "Overflow test for setPacket passed." << std::endl;
        
//         // Similarly, test getPacket for overflow.
//         try {
//             uint16_t outputData[5];
//             // Here, since 5*2 > 8, an overflow error is expected.
//             getPacket(packet, outputData, 5);
//             std::cerr << "Error: Expected overflow error in getPacket, but none was thrown." << std::endl;
//             assert(false);
//         } catch (const std::overflow_error &e) {
//             std::cout << "Caught expected exception for getPacket: " << e.what() << std::endl;
//         }
//         std::cout << "Overflow test for getPacket passed.\n" << std::endl;
//     }
    
//     // ---------------------------------------------------------
//     // Test 5: Null pointer / zero element test.
//     // Calling with a null pointer or zero dataSize should complete without errors.
//     {
//         std::cout << "Test 5: Null pointer / zero dataSize test" << std::endl;
//         byte packet[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//         // Set with zero elements: should simply clear the packet.
//         setPacket<int>(nullptr, 0, packet);
//         // The packet should have been zeroed-out.
//         for (size_t i = 0; i < 8; ++i) {
//             assert(packet[i] == 0);
//         }
        
//         int data[1] = {42};  // arbitrary data, will not be modified since dataSize is zero.
//         getPacket(packet, data, 0);  // Should not modify data.
//         assert(data[0] == 42);
//         std::cout << "Null pointer / zero dataSize test passed.\n" << std::endl;
//     }
    
//     std::cout << "All tests passed successfully." << std::endl;
//     return 0;
// }
