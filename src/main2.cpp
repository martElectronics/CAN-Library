#include <Arduino.h>
#include "MART_CAN.h"

CAN_BUS CAN(HardwareType::Transciever, 125, 1);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Nodo B ===");

    if (CAN.SetupState() != 0) {
        Serial.println("Error al inicializar CAN");
        while (1);
    }
}

void loop() {
    int valor[1];
    CAN.receive(); 
    CAN.printReceivedIds();
    if(CAN.getPacket(10, valor, 1)){
        Serial.println("ID5");
    } 
}
