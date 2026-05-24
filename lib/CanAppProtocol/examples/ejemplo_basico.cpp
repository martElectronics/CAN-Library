/**
 * @file ejemplo_basico.cpp
 * @brief Programa de ejemplo para la validación del protocolo de aplicación CanAppProtocol.
 * 
 * ==================================================================================
 * DESCRIPCIÓN GENERAL
 * ==================================================================================
 * Este programa demuestra cómo usar la librería `CanAppProtocol` sobre `MART_CAN`.
 * 
 * El código integra conceptualmente ambas partes del sistema en el mismo `loop()` consiste 
 * en:
 * - Nodo Emisor: Genera datos ficticios de celdas y los transmite en ráfagas.
 * - Nodo Receptor: Procesa los paquetes hardware al vuelo y reconstruye la estructura.
 * 
 * ==================================================================================
 * NOTAS DE DISEÑO
 * ==================================================================================
 * @note CRÍTICO (Gestión del Buffer): `MART_CAN` sobrescribe los paquetes en memoria 
 * si comparten el mismo ID. Por ello, el receptor drena el buffer hardware 
 * al vuelo (`CAN.receive()` + `CAN.getPacket()`) para evitar la pérdida de fragmentos.
 *
 * @note Salida de Datos: Una vez reconstruida la estructura con éxito, se formatea
 * y se vuelca por puerto serie (115200 bps) en formato CSV estructurado.
 * 
 * @author Eduardo Cantero Rascón
 * @date 17 de mayo de 2026
 */


#include <Arduino.h>
#include <MART_CAN.h>
#include <CanAppProtocol.h>

// 1. Define la estructura de datos real que quieres transmitir (puede ser lo que tú quieras)
struct DatosBateria {
    float stsVoltCells[12][11];
    float stsTempCells[12][9];
};

// Instancias de la estructura para envío y recepción
DatosBateria misDatosParaEnviar;
DatosBateria datosRecibidos;

// Un único ID de CAN para TODA la transmisión de la estructura de la batería
const uint32_t ID_BATERIA = 1171; // 0x493

// 2. Instanciamos el Protocolo de Aplicación apuntando a la estructura donde volcará los datos
CanAppProtocol canProtocol(ID_BATERIA, (uint8_t*)&datosRecibidos, sizeof(DatosBateria));

CAN_BUS CAN(HardwareType::Transciever, 500, 1, 4, 5, 80, 80);


 /**
 * @brief Genera un string en formato CSV con los datos de voltaje y temperatura. Resetea el estado.
 * 
 * @param stsVoltCells Array de 12x11 con los datos de voltaje.
 * @param stsTempCells Array de 12x9 con los datos de temperatura.
 * @return String El buffer con todos los datos formateados.
 */
String obtenerDatosCSV(float _stsVoltCells[12][11], float _stsTempCells[12][9]) {
  String buffer = "";
  
  // Estimación de tamaño: ~12 filas * ~150 caracteres por fila = 1800 bytes aprox.
  // Reservamos memoria para evitar lentitud y fragmentación.
  buffer.reserve(2000);

  // --- Filas de datos ---
  for (int i = 0; i < 12; i++) {
    // Nombre del módulo (M01, M02...)
    buffer += "M";
    int moduloNum = i + 1;
    if (moduloNum < 10) {
      buffer += "0";
    }
    buffer += moduloNum;

    // Datos de voltaje
    for (int j = 0; j < 11; j++) {
      buffer += ";";
      buffer += String(_stsVoltCells[i][j], 2); // 2 decimales
    }

    // Datos de temperatura
    for (int j = 0; j < 9; j++) {
      buffer += ";";
      buffer += String(_stsTempCells[i][j], 2); // 2 decimales
    }

    buffer += "\n";
  }

  return buffer;
}

void setup() {
    Serial.begin(115200);

    int can_init_attempts = 0;
    while (CAN.error == 1 && can_init_attempts < 10) {
        delay(100);
        can_init_attempts++;
    }
    
    if (CAN.error != 1) {
        Serial.println(F("CAN OK. Listo para enviar/recibir."));
    }
}

void loop() {
    // ------------------------------------------------------------
    // PARTE DE RECEPCIÓN (Típicamente estaría en la placa RX)
    // ------------------------------------------------------------
    
    // 1. Drenar completamente el buffer hardware procesándolos AL VUELO
    // Esto es CRÍTICO: MART_CAN sobrescribe los paquetes en memoria si tienen el mismo CAN ID.
    // Por tanto, por cada receive() debemos hacer un getPacket() inmediatamente.
    for (int i = 0; i < 80; i++) {
        CAN.receive(); // Baja 1 frame del hardware a la memoria interna
        
        // 2. Extraer del buffer si tiene nuestro ID esperado
        byte buffer[8];
        if (CAN.getPacket(ID_BATERIA, buffer, 8)) {
            size_t len_recibida = 0;

            // processReceivedPacket reconstruye la estructura automáticamente uniendo los fragmentos
            if (canProtocol.processReceivedPacket(ID_BATERIA, buffer, len_recibida)) {
                Serial.println(F("\n¡ESTRUCTURA COMPLETA RECIBIDA CON ÉXITO!"));
                Serial.print(F("Tamaño reconstruido: ")); Serial.print(len_recibida); Serial.println(F(" bytes."));
                
                // Ya podemos acceder a los datos de forma nativa:
                String data = obtenerDatosCSV(datosRecibidos.stsVoltCells, datosRecibidos.stsTempCells);
                Serial.print(data);
                Serial.println(F("#####################################################################\n"));
            }
        }
    }


    // ------------------------------------------------------------
    // PARTE DE ENVÍO (Típicamente estaría en la placa TX)
    // ------------------------------------------------------------
    static unsigned long lastSend = 0;
    if (millis() - lastSend > 2000) {
        lastSend = millis();
        
        // Rellenar con algunos datos de prueba
        // Simulamos voltajes entre 3.00V y 4.20V (típico de celdas de Litio)
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 11; j++) {
                // esp_random() da un número enorme, usamos el módulo (%) para acotar el rango
                float voltajeAleatorio = 3.0f + (float)(esp_random() % 121) / 100.0f; 
                misDatosParaEnviar.stsVoltCells[i][j] = voltajeAleatorio;
            }
        }

        // Simulamos temperaturas entre 22.0°C y 45.0°C
        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 9; j++) {
                float tempAleatoria = 22.0f + (float)(esp_random() % 231) / 10.0f;
                misDatosParaEnviar.stsTempCells[i][j] = tempAleatoria;
            }
        }

        Serial.println(F("\nIniciando transmisión de la estructura de batería..."));
        
        // sendData cogerá sizeof(DatosBateria) (aprox 960 bytes) y lo enviará en ~138 mensajes CAN
        // separados por 2ms de forma automática.
        if (canProtocol.sendData(CAN, (uint8_t*)&misDatosParaEnviar, sizeof(DatosBateria), 2)) {
            Serial.println(F("Transmisión finalizada correctamente."));
        } else {
            Serial.println(F("Error en la transmisión (bus saturado o error de hardware)."));
            CAN.rebootBusFromError(); // Intentar recuperación
        }
    }
    
    delay(10); // Loop principal muy rápido
}
