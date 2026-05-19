//Este código implementa nuevos métodos setPacket y getPacket más simples. Sólo se contempla un tipo posible
// de array a la entrada/salida


#include <iostream>
#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>
#include <typeinfo>
#include <limits>

using byte = unsigned char;

/**
 * Serializes data into an 8-byte packet.
 * 
 * @param data Pointer to the data array to serialize
 * @param dataSize Number of elements in the data array
 * @param outputArray The output 8-byte array
 * @throws std::overflow_error If the data exceeds 8 bytes
 */
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
    
    // Copy the data
    if (data != nullptr && dataSize > 0) {
        std::memcpy(outputArray, data, dataBytes);
    }
}

/**
 * Deserializes data from an 8-byte packet.
 * 
 * @param inputArray The input 8-byte array
 * @param data Pointer to the data array to populate
 * @param dataSize Number of elements expected in the data array
 * @throws std::overflow_error If trying to extract more data than available
 */
template<typename T>
void getPacket(const byte inputArray[], T* data, size_t dataSize) {
    // Calculate size
    size_t dataBytes = sizeof(T) * dataSize;
    
    // Check for overflow
    if (dataBytes > 8) {
        throw std::overflow_error("Packet overflow: trying to extract more data than available");
    }
    
    // Extract the data
    if (data != nullptr && dataSize > 0) {
        std::memcpy(data, inputArray, dataBytes);
    }
}

// Utility function to print a byte array in hexadecimal format
void printByteArray(const byte array[], size_t length) {
    std::cout << "[ ";
    for (size_t i = 0; i < length; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(array[i]);
        if (i < length - 1) {
            std::cout << ", ";
        }
    }
    std::cout << " ]" << std::dec << std::endl;
}

// Helper function to generate random values for test data
template<typename T>
void generateRandomValues(T* array, size_t size) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Use regular if statements instead of constexpr if for C++14 compatibility
    if (std::is_same<T, bool>::value) {
        std::uniform_int_distribution<> distrib(0, 1);
        for (size_t i = 0; i < size; ++i) {
            array[i] = static_cast<bool>(distrib(gen));
        }
    } else if (std::is_integral<T>::value) {
        if (std::is_signed<T>::value) {
            std::uniform_int_distribution<typename std::conditional<sizeof(T) < sizeof(int), int, T>::type> 
                distrib(std::numeric_limits<T>::min() / 2, std::numeric_limits<T>::max() / 2);
            for (size_t i = 0; i < size; ++i) {
                array[i] = static_cast<T>(distrib(gen));
            }
        } else {
            std::uniform_int_distribution<typename std::conditional<sizeof(T) < sizeof(unsigned int), unsigned int, T>::type> 
                distrib(0, std::numeric_limits<T>::max() / 2);
            for (size_t i = 0; i < size; ++i) {
                array[i] = static_cast<T>(distrib(gen));
            }
        }
    } else if (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> distrib(-1000.0, 1000.0);
        for (size_t i = 0; i < size; ++i) {
            array[i] = distrib(gen);
        }
    } else {
        throw std::runtime_error("Unsupported type for random generation");
    }
}

// Run a basic test for a specific type and array size
template<typename T>
bool runBasicTest(const char* typeName, size_t arraySize, bool expectSuccess = true) {
    std::cout << "\nTest: " << typeName << " array (size " << arraySize << ")" << std::endl;
    
    byte packet[8] = {0};
    T* data = new T[arraySize];
    T* newData = new T[arraySize];
    
    // Generate random test data
    generateRandomValues(data, arraySize);
    
    bool success = false;
    try {
        // Print original values
        std::cout << "Original values: data=[";
        for (size_t i = 0; i < arraySize; ++i) {
            if (std::is_same<T, char>::value || std::is_same<T, unsigned char>::value) {
                std::cout << static_cast<int>(data[i]);
            } else {
                std::cout << data[i];
            }
            if (i < arraySize - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        
        // Serialize
        setPacket(data, arraySize, packet);
        std::cout << "Serialized packet: ";
        printByteArray(packet, 8);
        
        // Deserialize
        std::memset(newData, 0, sizeof(T) * arraySize);
        getPacket(packet, newData, arraySize);
        
        // Print deserialized values
        std::cout << "Deserialized values: data=[";
        for (size_t i = 0; i < arraySize; ++i) {
            if (std::is_same<T, char>::value || std::is_same<T, unsigned char>::value) {
                std::cout << static_cast<int>(newData[i]);
            } else {
                std::cout << newData[i];
            }
            if (i < arraySize - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        
        // Verify
        success = true;
        for (size_t i = 0; i < arraySize; ++i) {
            success &= (data[i] == newData[i]);
        }
        
        if (expectSuccess) {
            std::cout << "Test " << (success ? "PASSED" : "FAILED") << std::endl;
        } else {
            std::cout << "Test FAILED: Expected overflow exception but none was thrown" << std::endl;
            success = false;
        }
    } catch (const std::overflow_error& e) {
        if (expectSuccess) {
            std::cout << "Test FAILED: Unexpected exception: " << e.what() << std::endl;
            success = false;
        } else {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
            std::cout << "Test PASSED" << std::endl;
            success = true;
        }
    } catch (const std::exception& e) {
        std::cout << "Test FAILED: Unexpected exception: " << e.what() << std::endl;
        success = false;
    }
    
    delete[] data;
    delete[] newData;
    return success;
}

// Test for packet corruption resistance
template<typename T>
bool runCorruptionTest(const char* typeName, size_t arraySize) {
    std::cout << "\nCorruption Test: " << typeName << " array (size " << arraySize << ")" << std::endl;
    
    byte packet[8] = {0};
    T* data = new T[arraySize];
    T* origData = new T[arraySize];
    T* newData = new T[arraySize];
    
    // Generate random test data
    generateRandomValues(data, arraySize);
    std::memcpy(origData, data, sizeof(T) * arraySize);
    
    bool success = false;
    try {
        // Serialize
        setPacket(data, arraySize, packet);
        std::cout << "Original packet: ";
        printByteArray(packet, 8);
        
        // Corrupt a byte in the middle of the packet
        byte originalByte = packet[4];
        packet[4] = ~packet[4]; // Flip all bits
        std::cout << "Corrupted packet: ";
        printByteArray(packet, 8);
        
        // Deserialize
        std::memset(newData, 0, sizeof(T) * arraySize);
        getPacket(packet, newData, arraySize);
        
        // Check for corruption
        bool dataCorrupted = false;
        for (size_t i = 0; i < arraySize; ++i) {
            if (origData[i] != newData[i]) {
                dataCorrupted = true;
                break;
            }
        }
        
        if (dataCorrupted) {
            std::cout << "Corruption detected as expected" << std::endl;
            std::cout << "Test PASSED" << std::endl;
            success = true;
        } else {
            std::cout << "Test FAILED: Data corruption not detected" << std::endl;
            success = false;
        }
    } catch (const std::exception& e) {
        std::cout << "Test FAILED: Unexpected exception: " << e.what() << std::endl;
        success = false;
    }
    
    delete[] data;
    delete[] origData;
    delete[] newData;
    return success;
}

// Test boundary cases
template<typename T>
bool runBoundaryTest() {
    std::cout << "\nBoundary Test for " << typeid(T).name() << std::endl;
    
    byte packet[8] = {0};
    bool success = true;
    
    // Calculate how many elements can fit
    size_t maxElements = 8 / sizeof(T);
    
    std::cout << "Maximum " << typeid(T).name() << " elements that can fit: " << maxElements << std::endl;
    
    // Test exactly at the boundary
    bool boundarySuccess = runBasicTest<T>("Boundary", maxElements);
    success &= boundarySuccess;
    
    // Test one over the boundary
    bool overflowSuccess = runBasicTest<T>("Overflow", maxElements + 1, false);
    success &= overflowSuccess;
    
    return success;
}

// Test empty data
bool runEmptyTest() {
    std::cout << "\nEmpty Data Test" << std::endl;
    
    byte packet[8] = {0};
    
    try {
        // Serialize with no data
        setPacket(static_cast<int*>(nullptr), 0, packet);
        std::cout << "Serialized packet (empty): ";
        printByteArray(packet, 8);
        
        // Check if packet is all zeros
        bool allZeros = true;
        for (int i = 0; i < 8; i++) {
            if (packet[i] != 0) {
                allZeros = false;
                break;
            }
        }
        
        std::cout << "Test " << (allZeros ? "PASSED" : "FAILED") << std::endl;
        return allZeros;
    } catch (const std::exception& e) {
        std::cout << "Test FAILED: Unexpected exception: " << e.what() << std::endl;
        return false;
    }
}

// Test consecutive operations (serialize/deserialize multiple times)
template<typename T>
bool runConsecutiveOperationsTest(const char* typeName, size_t arraySize) {
    std::cout << "\nConsecutive Operations Test: " << typeName << " array" << std::endl;
    
    byte packet1[8] = {0};
    byte packet2[8] = {0};
    T* data1 = new T[arraySize];
    T* data2 = new T[arraySize];
    T* newData1 = new T[arraySize];
    T* newData2 = new T[arraySize];
    
    // Generate random test data
    generateRandomValues(data1, arraySize);
    generateRandomValues(data2, arraySize);
    
    bool success = false;
    try {
        // First operation
        setPacket(data1, arraySize, packet1);
        std::cout << "First packet: ";
        printByteArray(packet1, 8);
        
        // Second operation
        setPacket(data2, arraySize, packet2);
        std::cout << "Second packet: ";
        printByteArray(packet2, 8);
        
        // Deserialize first packet
        getPacket(packet1, newData1, arraySize);
        
        // Deserialize second packet
        getPacket(packet2, newData2, arraySize);
        
        // Verify first packet
        bool success1 = true;
        for (size_t i = 0; i < arraySize; ++i) {
            success1 &= (data1[i] == newData1[i]);
        }
        
        // Verify second packet
        bool success2 = true;
        for (size_t i = 0; i < arraySize; ++i) {
            success2 &= (data2[i] == newData2[i]);
        }
        
        success = success1 && success2;
        std::cout << "First packet recovery: " << (success1 ? "PASSED" : "FAILED") << std::endl;
        std::cout << "Second packet recovery: " << (success2 ? "PASSED" : "FAILED") << std::endl;
        std::cout << "Test " << (success ? "PASSED" : "FAILED") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test FAILED: Unexpected exception: " << e.what() << std::endl;
        success = false;
    }
    
    delete[] data1;
    delete[] data2;
    delete[] newData1;
    delete[] newData2;
    return success;
}

// Run all tests
void runComprehensiveTests() {
    std::cout << "Running comprehensive tests for packet serialization and deserialization..." << std::endl;
    
    // Print size information
    std::cout << "Size of int8_t: " << sizeof(int8_t) << " bytes" << std::endl;
    std::cout << "Size of int16_t: " << sizeof(int16_t) << " bytes" << std::endl;
    std::cout << "Size of int32_t: " << sizeof(int32_t) << " bytes" << std::endl;
    std::cout << "Size of int64_t: " << sizeof(int64_t) << " bytes" << std::endl;
    std::cout << "Size of float: " << sizeof(float) << " bytes" << std::endl;
    std::cout << "Size of double: " << sizeof(double) << " bytes" << std::endl;
    std::cout << "Size of bool: " << sizeof(bool) << " bytes" << std::endl;
    std::cout << std::endl;
    
    int testsPassed = 0;
    int totalTests = 0;
    
    // Test categories
    std::cout << "==== BASIC FUNCTIONALITY TESTS ====" << std::endl;
    // Test various types and sizes
    totalTests++;
    if (runBasicTest<uint8_t>("uint8_t", 8)) testsPassed++;
    
    totalTests++;
    if (runBasicTest<int8_t>("int8_t", 8)) testsPassed++;
    
    totalTests++;
    if (runBasicTest<int16_t>("int16_t", 4)) testsPassed++;
    
    totalTests++;
    if (runBasicTest<uint16_t>("uint16_t", 4)) testsPassed++;
    
    totalTests++;
    if (runBasicTest<int32_t>("int32_t", 2)) testsPassed++;
    
    totalTests++;
    if (runBasicTest<float>("float", 2)) testsPassed++;
    
    totalTests++;
    if (runBasicTest<bool>("bool", 8)) testsPassed++;
    
    totalTests++;
    if (runBasicTest<double>("double", 1)) testsPassed++;
    
    std::cout << "\n==== BOUNDARY AND EDGE CASE TESTS ====" << std::endl;
    // Test boundary cases
    totalTests++;
    if (runBoundaryTest<uint8_t>()) testsPassed++;
    
    totalTests++;
    if (runBoundaryTest<int16_t>()) testsPassed++;
    
    totalTests++;
    if (runBoundaryTest<int32_t>()) testsPassed++;
    
    totalTests++;
    if (runBoundaryTest<int64_t>()) testsPassed++;
    
    // Test empty data
    totalTests++;
    if (runEmptyTest()) testsPassed++;
    
    std::cout << "\n==== DATA INTEGRITY TESTS ====" << std::endl;
    // Test data corruption
    totalTests++;
    if (runCorruptionTest<int16_t>("int16_t", 4)) testsPassed++;
    
    // Test consecutive operations
    totalTests++;
    if (runConsecutiveOperationsTest<uint8_t>("uint8_t", 8)) testsPassed++;
    
    // Print summary
    std::cout << "\n==== TEST SUMMARY ====" << std::endl;
    std::cout << "Tests passed: " << testsPassed << " out of " << totalTests << std::endl;
    std::cout << "Success rate: " << (static_cast<double>(testsPassed) / totalTests * 100.0) << "%" << std::endl;
    
    if (testsPassed == totalTests) {
        std::cout << "\nALL TESTS PASSED! The implementation is working correctly." << std::endl;
    } else {
        std::cout << "\nSome tests failed. Please check the implementation." << std::endl;
    }
}

int main() {
    runComprehensiveTests();
    return 0;
}