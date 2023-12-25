<img width="823" alt="Screenshot 2023-12-25 at 18 13 10" src="https://github.com/martElectronics/can-library/assets/148893488/6432a10b-65b5-4635-a6b5-8c0c5713b629"># can-library
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




# Acknowledgments
The MART_CAN library wouldn't be possible without the contributions of the open-source community. Special thanks to all the contributors who have helped make this library possible.

