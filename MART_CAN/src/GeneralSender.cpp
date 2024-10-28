#include <Arduino.h>
#include <MART_CAN.h>

CAN_BUS CAN(5);

void setup()
{
    Serial.begin(9600);

    unsigned long removeableIDs[2] = {101, 103};

    // Configura la memoria donde se guardan los datos recibidos para que borre todos los paquetes
    // que son procesados con getPacket()
    CAN.DataIN.setRemovableIds();

    // Configura la memoria donde se guardan los datos a enviar para que borre los paquetes con ID=101 e ID=103
    // una vez que son enviados con send()
    CAN.DataOUT.setRemovableIds(removeableIDs, 2);
}

void loop()
{

    bool dataBool16[16] = {1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1};
    short dataShort1[1] = {12}, dataShort2[2] = {12, -1148};
    int dataInt1[1] = {-3}, dataInt2[2] = {-3, 1234567};
    float dataFloat1[1] = {3.142592};

    // Se crea un paquete con ID=100 configurado como RRF (petición de datos remota). Los paquetes que estén configurados
    // en los receptores responderán enviando información cuando reciban este paquete
    CAN.setPacket(100);

    // Se procesan los 8 bytes de dataInt2 como "int" (de 4 en 4) y se crea un paquete de ID=101 con esos datos
    CAN.setPacket(101, dataInt2);

    // Se procesan los 4 bytes de dataShort2 como "short"(de 2 en 2), los 4 bytes de dataInt1 como "int" y se crea
    // un paquete de ID=102 con esos datos
    CAN.setPacket(102, dataShort2, dataInt1);

    // Se crea un paquete que contiene información de tipo "bool", "float" y "short"
    CAN.setPacket(200, dataBool16, dataFloat1, dataShort1);

    // Lectura de entradas físicas del microcontrolador
    bool condicion1 = digitalRead(10);
    bool condicion2 = analogRead(14) > 2048;

    // UTILIZAR SÓLO UNA DE LAS DOS OPCIONES
    // OPCIÓN 1: Enviar datos por el bus sólo si se cumplen unas condiciones determinadas
    if (condicion1)
        CAN.send(100);
    else if (condicion2)
        CAN.send(101);
    else
    {
        CAN.send(101);
        CAN.send(200);
    }

    // OPCIÓN 2 : Enviar todos los datos guardados en memoria
    CAN.send();
}
