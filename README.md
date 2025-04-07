# CAN_Library
MART CAN Library

The MART_CAN library is an Arduino library designed to facilitate CAN (Controller Area Network) communication and data packet storage. It simplifies the process of sending and receiving CAN messages, as well as handling different data types in your Arduino projects.

# Features
-CAN_BUS Class: Central component for managing CAN communication using an MCP2515 CAN controller. It includes functions for initialization, sending data, receiving data, packet handling, data packing, and data unpacking.

-CAN_DATA Class: Manages the storage and organization of CAN data packets. Features include packet storage, adding packets, removing packets, retrieving packets by ID, iterating over packets, and more.

-Integration with MCP_CAN Library: Compatible with the MCP_CAN library, making it suitable for a wide range of Arduino boards and CAN setups.

-Data Packet Management: Simplify the handling of CAN data packets with a well-structured data storage system. Store, retrieve, and manipulate CAN packets effortlessly.

-Multiple Data Types: Support for various data types, including integers, floats, and boolean arrays, ensures compatibility with a wide range of data formats.

-Packing and Unpacking: Efficiently pack and unpack data into/from CAN packets, making it simple to work with complex data structures.

# Getting Started
-Create a new PlatformIO project, then add the /lib and platformio.ini files to the project. Please add /src examples as needed.

-For uploading, select the <example>.cpp environment and hit the upload arrow button

<img width="823" alt="Screenshot 2023-12-25 at 18 13 10" src="https://github.com/martElectronics/can-library/assets/148893488/51bbe785-042d-48f7-be75-f78f2ec8f644">

# Implementation on code
<h2>Constructor</h2>
<span>- Besides the other constructors, the recommended one is:
  -CAN_BUS(HardwareType type, unsigned int speed, int _nodeID, int pinCs = 0, int8_t TX = 5, int8_t RX = 4, uint16_t txQueue = 10, uint16_t rxQueue=10)
  
    -type -> Enum for chossing between Transciever or Controller Modules (Transciver Not supported for Arduino)
    -speed -> Enum defined CAN_SPEED_500, CAN_SPEED_1000 for KBs speed on bus
    -_nodeID -> Id for node in the can bus
    -pinCs -> Variable for configuring CS pin on Controller
    -TX -> TX pin for esp & transciever
    -RX -> RX pin for esp & transciever
    -txQueue & rxQueue -> Transmit and Recieve queue sizes
>[!Note]
> It is possible to change transmission speed, queue sizes and timeout time while running, you must call
  
      setupCANHardware(unsigned int speed, uint16_t txQueue = 10, uint16_t rxQueue = 10, int timeoutRead = 100)
</span>

<h2>On Setup()</h2>
<span>
  <h3>If ESP32</h3>
    
    - Change SPI pins if needed
    - Declare constructor CAN_BUS (Change TX & RX pins if needed)
    - Check CAN_BUS variable error for problems at inicialitation 
    - Check timeout variable for reading messages if using Transciever (default 100ms) 
  
  <h3>If Arduino</h3>
</span>

>[!Warning]
>There is no support for Transciever on Arduino microcontrollers
>

<h2>Platform.IO file</h2>
<span>
    For PlatformIO files, you must use different configurations depending on the microcontroller used 
    Use this as reference, and add the necesary additional configurations

    [env]
    framework = arduino
    build_flags = -std=c++11
    build_unflags = -std=gnu++17
    monitor_speed = 115200
    lib_deps = 
        autowp/autowp-mcp2515@^1.0.3
    
    
>[!Note]
>At the moment there are only 3 options used on the car:

> ESP32:

          [env:ESP32]
          platform = espressif32
          board = esp32dev
          build_src_filter = +<main11.cpp> 
          build_flags = ${env.build_flags}
                          -D ESP32
          upload_port = /dev/ttyUSB1  #OR COM5, etc check for used USB port 

> ESP-S3
      Change the above one with the following

       board = esp32-s3-devkitc-1
       build_flags = ${env.build_flags} 
                     -D ESP32S3

> ARDUINO

        [env:arduino]
        platform = atmelavr # may be necessary to change this too depending on the model
        board = nanoatmega328new # Or the one needed
        build_src_filter = +<main.cpp>
        build_flags = ${env.build_flags}
                        -D ARDUINO_MICRO
        lib_deps =
            ${env.lib_deps}
            https://github.com/mike-matera/ArduinoSTL.git
        upload_port = /dev/ttyUSB0


>[!Important]
>Build Flags seen above are mandatory for the correct execution of the code. they are precompile flags necessary for the correct importantion of complementary libraries. 
</span>


</span>

# Acknowledgments
The MART_CAN library wouldn't be possible without the contributions of the open-source community. Special thanks to all the contributors who have helped make this library possible.

