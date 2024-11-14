//Ejemplo de uso de la función millis() como alternativa a delay() para las rutinas que requieran de una temporización
//millis() es una función que devuelve el tiempo en milisegundos que ha transcurrido desde que se ha
//iniciado el microcontrolador.

//Parpadeo de un led para que esté 2 segundos encendido y 1 segundo apagado

#include <Arduino.h>

unsigned long tiempoAnterior;
unsigned tiempoON=2000, tiempoOFF=1000;
bool estadoLed;
unsigned pinLed=16;
void setup()
{
  pinMode(pinLed,OUTPUT);
  tiempoAnterior=millis();
}

void loop()
{
  if(((millis()-tiempoAnterior)>=tiempoON) && estadoLed)
  {
    estadoLed=true;
    tiempoAnterior=millis();
  }
  if(((millis()-tiempoAnterior)>=tiempoOFF) && !estadoLed)
  {
    estadoLed=false;
    tiempoAnterior=millis();
  }

  digitalWrite(pinLed,estadoLed);
}



