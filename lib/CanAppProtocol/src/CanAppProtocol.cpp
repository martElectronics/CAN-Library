#include "CanAppProtocol.h"
#include <cstring>

CanAppProtocol::CanAppProtocol(uint32_t canId, uint8_t* rxBuffer, size_t maxLen) {
    _canId = canId;
    _rxBuffer = rxBuffer;
    _rxMaxLen = maxLen;
    _rxCurrentLen = 0;
    _expectedSeq = 0;
    _receiving = false;
}

bool CanAppProtocol::sendData(CAN_BUS &CAN, const uint8_t *data, size_t length, uint32_t delayMs) {
    if (length == 0 || length > 1785) { // 255 paquetes max * 7 bytes = 1785 bytes soportados
        return false; 
    }

    size_t bytesSent = 0;
    uint8_t seqNum = 0;

    // 1. Enviar los paquetes fragmentados de datos
    while (bytesSent < length) {
        uint8_t frameBuffer[8];
        memset(frameBuffer, 0, 8); // Llenar con ceros por defecto

        // Calcular cuántos bytes meter en este fragmento (máx 7)
        size_t bytesToSend = length - bytesSent;
        if (bytesToSend > 7) {
            bytesToSend = 7;
        }

        // Multiplexor (Secuencia)
        frameBuffer[0] = seqNum;
        
        // Cargar los datos
        memcpy(&frameBuffer[1], &data[bytesSent], bytesToSend);

        // Transmitir al bus CAN
        if (!CAN.setPacket(_canId, frameBuffer, 8, false)) {
            return false; // Falló al cargar en buffer
        }
        if (!CAN.send(_canId)) {
            return false; // Falló envío hardware
        }

        bytesSent += bytesToSend;
        seqNum++;
        
        // Retardo para no sobrecargar las colas hardware del emisor o receptor
        if (delayMs > 0) {
            delay(delayMs);
        }
    }

    // 2. Enviar el paquete de Fin de Trama (EOF)
    uint8_t eofBuffer[8];
    memset(eofBuffer, 0, 8);
    eofBuffer[0] = 0xFF; // Flag EOF (Fin de la ráfaga)
    eofBuffer[1] = (uint8_t)(length & 0xFF);        // LSB de longitud
    eofBuffer[2] = (uint8_t)((length >> 8) & 0xFF); // MSB de longitud

    if (!CAN.setPacket(_canId, eofBuffer, 8, false)) {
        return false;
    }
    if (!CAN.send(_canId)) {
        return false;
    }

    return true; // Transmisión completada
}

bool CanAppProtocol::processReceivedPacket(uint32_t id, const byte* frameData, size_t &outLength) {
    if (id != _canId) {
        return false; // Descartar IDs que no sean el esperado
    }

    uint8_t seqNum = frameData[0]; // Byte 0 = Multiplexor

    // Verificar si es el paquete final (End Of Frame)
    if (seqNum == 0xFF) {
        if (_receiving) {
            // Verificar la longitud notificada en el EOF
            uint16_t expectedTotalLength = frameData[1] | (frameData[2] << 8);
            
            outLength = expectedTotalLength;
            // Evitamos overflow por seguridad si enviaron más del MAX local
            if (outLength > _rxMaxLen) {
                outLength = _rxMaxLen;
            }
            
            _receiving = false; // Paramos recepción
            return true;        // ¡Estructura completa disponible para el usuario!
        }
        return false; // Recibimos EOF sin estar en medio de una recepción, ignoramos.
    }

    // Si es el primer paquete (secuencia 0), inicializamos la reconstrucción
    if (seqNum == 0) {
        resetReception();
        _receiving = true;
    }

    // Comprobar desincronización (si se ha perdido un paquete intermedio)
    if (seqNum != _expectedSeq) {
        resetReception(); // Abortar recepción actual
        return false;
    } else if (!_receiving) {
        return false;
    }

    // Copiar la información al buffer final de la estructura
    size_t bytesToCopy = 7;
    // Precaución para no exceder los límites físicos del buffer del receptor
    if (_rxCurrentLen + bytesToCopy > _rxMaxLen) {
        bytesToCopy = _rxMaxLen - _rxCurrentLen; 
    }

    if (bytesToCopy > 0) {
        memcpy(&_rxBuffer[_rxCurrentLen], &frameData[1], bytesToCopy);
        _rxCurrentLen += bytesToCopy;
    }

    _expectedSeq++;

    return false; // Todavía no hemos terminado, seguimos procesando
}

void CanAppProtocol::resetReception() {
    _rxCurrentLen = 0;
    _expectedSeq = 0;
    _receiving = false;
}
