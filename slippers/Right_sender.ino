
#include <SPI.h>
#include "RF24.h"
const int sensor[3] = {A0, A1, A2}; //An array define the analog ports connected to the sensors
int valueR[3];
byte valueR1[3];


RF24 radio(10,9);
const uint64_t rAddress[] = {0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL };

void setup() {
  Serial.begin(115200);
  
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);

  radio.openWritingPipe(rAddress[1]);
  Serial.println(F("Initiated."));
}

void loop(){
    //Write the values of sensors into the buffer
    getSensorValue();
    intToByte();


  Serial.println(F("Now sending:"));
  radio.write(&valueR1, sizeof(byte[3]));
  Serial.println(valueR[0]);
  Serial.println(valueR[1]);
  Serial.println(valueR[2]);

  
  delay(1000);
}

void getSensorValue(){

  valueR[0] = analogRead(sensor[0]);
  valueR[1] = analogRead(sensor[1]);
  valueR[2] = analogRead(sensor[2]);
  
  fifo();
  //Mapping the values to a 0-99 scale, place the current sensors readings to the corresponding array.
  for(int i=0;i<=2;++i){
    valueR[i]=RangeMap(valueR[i]);
    }
}
 
void intToByte(){
  byte valueR1[3];
  for(int a=0; a<3; a++){
    valueR1[a] =valueR[a]-'0';
  }
}
/*
 * Make TWO buffer array First In First Out
 * Parameter: None (global array used in this function)
 * Return: sensorValues as global variable array
 * Version: 1.0
 */
void fifo(){
    for(int b=3;b>=1;--b){
      valueR[b]=valueR[b-1];
      }
    }
/*
 * Limit and map the data value to 0-99, and eliminate floating around 0
 * Parameter: data ranged from 0-1023
 * Return: data ranged from 0-99
 * Version: 1.0

 */
 
int RangeMap(int a){
  a=constrain(a,0,1023);
  a=map(a,0,1023,99,-2);
  a=constrain(a,0,99);
  return a;
  }
