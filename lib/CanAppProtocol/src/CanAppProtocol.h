#ifndef CAN_APP_PROTOCOL_H
#define CAN_APP_PROTOCOL_H

#include <Arduino.h>
#include <MART_CAN.h>

/**
 * @class CanAppProtocol
 * @brief Protocolo de capa de aplicación para CAN Bus (Multiplexado de datos grandes).
 * 
 * Permite enviar cualquier estructura o bloque de datos de hasta 1785 bytes
 * a través de un único identificador CAN nativo. Automáticamente divide
 * los datos en el emisor y los reensambla en el receptor, manejando las secuencias.
 * 
 * Formato del mensaje de datos:
 * - Byte 0: Número de secuencia (0 a 254)
 * - Bytes 1 a 7: Datos útiles (7 bytes por paquete)
 * 
 * Formato del mensaje de Fin de Trama (EOF):
 * - Byte 0: 0xFF (255) - Indicador de fin de transmisión
 * - Bytes 1-2: Longitud total de los datos transmitidos en bytes (uint16_t, Little Endian)
 * - Bytes 3-7: Reservado (0)
 * 
 * @author Eduardo Cantero Rascón
 * @date 17 de mayo de 2026
 */
class CanAppProtocol {
private:
    uint32_t _canId;
    uint8_t* _rxBuffer;
    size_t _rxMaxLen;
    size_t _rxCurrentLen;
    uint8_t _expectedSeq;
    bool _receiving;

public:
    /**
     * @brief Constructor del protocolo para el receptor.
     * 
     * @param canId El único identificador CAN que se utilizará en la transmisión (ej. 0x490).
     * @param rxBuffer Puntero al buffer o estructura donde se reconstruirán los datos recibidos.
     * @param maxLen Tamaño máximo en bytes de dicho buffer (típicamente sizeof(TuEstructura)).
     */
    CanAppProtocol(uint32_t canId, uint8_t* rxBuffer, size_t maxLen);

    /**
     * @brief Envía un buffer o estructura arbitraria a través del CAN Bus.
     * 
     * Fragmentará los datos y enviará todos los mensajes bajo el mismo canId.
     * 
     * @param CAN Instancia de la librería MART_CAN.
     * @param data Puntero a la estructura o buffer a enviar: (uint8_t*)&miEstructura
     * @param length Tamaño total de la estructura en bytes: sizeof(miEstructura)
     * @param delayMs Retraso en milisegundos entre cada fragmento para evitar saturación del bus.
     * 
     * @return true si la estructura se envió por completo exitosamente.
     */
    bool sendData(CAN_BUS &CAN, const uint8_t *data, size_t length, uint32_t delayMs = 2);

    /**
     * @brief Procesa un paquete CAN entrante y reconstruye la estructura internamente.
     * 
     * Debería llamarse cada vez que se detecte un paquete con el canId esperado.
     * 
     * @param id Identificador CAN del mensaje recibido.
     * @param frameData Los 8 bytes puros del frame CAN (payload).
     * @param outLength Si la función devuelve true, contendrá el tamaño real de la estructura recibida.
     * 
     * @return true SÓLO cuando se ha recibido el paquete EOF y la estructura completa está lista en rxBuffer.
     */
    bool processReceivedPacket(uint32_t id, const byte* frameData, size_t &outLength);

    /**
     * @brief Resetea la máquina de estados de recepción. 
     * Se llama automáticamente si hay un error de sincronización, pero se puede llamar manual.
     */
    void resetReception();
};

#endif // CAN_APP_PROTOCOL_H
