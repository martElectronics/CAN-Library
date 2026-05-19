#include <Arduino.h>
#include "MART_CAN.h"

/*Construtor*/
/*
* El primer parametro siempre es el mismo
* EL segundo es la velocidad, solemos usar 125k bits/s pero se puede aumentar si necesitamos mayor
* capacidad de envio
* El tercero es la id del nodo, esto configura el filtro de IDs por hardware, el nodo 2 (pdm) 
* y 4 (VCU) recibira solo las ids que son necesarias en dicha placa, el nodo 3 (bms) no recibe 
* ninguna id. Cualquier otro numero recibira todas las ids
*/
CAN_BUS CAN(HardwareType::Transciever, 125, 1);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== Nodo A ===");

    if (CAN.SetupState() != 0) {
        Serial.println("Error al inicializar CAN");
        while (1);
    }
    Serial.println("Bus CAN listo.");

    /*Configura los tiempos de envio de todos las ids usadas (solo las definidas en can_data)*/
    CAN.configurePacketTimersByPriority(); 
}

void loop() {

    /****************Envio de mensajes*******************/
    //Hasta 8 bytes (cada int ocupa 4 bytes, los bool 1)
    
    int valor[1] = {27}; //datos a enviar

    if (!CAN.setPacket(10, valor, 1)) { //id paquete / contenido / tamaño array
        Serial.println("Error al crear el paquete");
    }
    if (CAN.send()) {
        Serial.println("Mensaje enviado");
    }else{
        Serial.println("Error al enviar ");
    }
    
    /****************Recepción de mensajes*******************/
    
    bool valor2[8]; //donde se guardaran los datos recibidos

    if(CAN.getPacket(10, valor2, 8)){ //los paquetes son borrados de la cola nada más ser leidos
        Serial.println("Paquete recibido de la id 10: ");
        CAN.printArray(valor2);
    }


    /*Comprobación de que el bus no ha entrado en estado de fallo y reiniciarlo si es necesario*/
    CAN.rebootBusFromError(); 

    //delay(10); //Importante no meter delay que rompa la temporización de paquetes
}
