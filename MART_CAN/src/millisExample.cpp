
#include <Arduino.h>
#include <MART_CAN.h>

CAN_BUS CAN(5,1);
int dataInt1[1];
int dataInt11[1]={666};
int dataInt2[2];
int delayData[2];
int cont=0;

unsigned long int firstId=7;
unsigned long int lastId=900;
unsigned long int numIds=10;

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    unsigned long int timeAux=millis();
     unsigned numIds = map(analogRead(14),0,4095,0,1000);
    dataInt2[0]=cont;
    
    CAN.receive();
   
    CAN.setPacket(firstId,dataInt2);
    
    delay(numIds);
    CAN.send();
 
   // CAN.printStatusData(1);

    // cont++;

}

/*   dataInt2[0]=cont;
    dataInt2[1]=potValue;
    dataInt1[0]=cont;
    cont++;*/
