#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS CAN(HardwareType::Transciever, 125, 0);  // 125 kbps para coincidir con la ESP32

void setup() {
    Serial.begin(115200);
    delay(2000);  // Tiempo suficiente para abrir el monitor serie
    Serial.println("=== Nodo A (STM32 Emisor) ===");

#if defined(STM32G4xx)
    // Verificar el clock real del FDCAN para confirmar el bit timing
    uint32_t fdcanClk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_FDCAN);
    Serial.print("FDCAN CLK real: ");
    Serial.print(fdcanClk / 1000000);
    Serial.println(" MHz");
    // Con 125k y Prescaler=12, TimeSeg1=12, TimeSeg2=3 (BTQ=16):
    // Baudrate = fdcanClk / (Prescaler * BTQ) = fdcanClk / (12 * 16)
    Serial.print("Baudrate calculado: ");
    Serial.print(fdcanClk / (12 * 16));
    Serial.println(" bps  (esperado: 125000)");
#endif

    if (CAN.SetupState() != 0) {
        Serial.println("ERROR: fallo al inicializar CAN");
        while (1);
    }
    Serial.println("Bus CAN listo a 125 kbps.");
}

void loop() {
    int valor[1] = {27};

    if (!CAN.setPacket(10, valor, 1)) {
        Serial.println("Error al crear el paquete");
    }
    if (CAN.send()) {
        Serial.println("Mensaje enviado OK (ID=10, val=27)");
    } else {
        Serial.println("Error al enviar");
    }
    delay(500);  // No saturar el bus
}
