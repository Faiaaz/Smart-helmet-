#include<SPI.h>
#include "RF24.h"
#include <SharpIR.h>

#define ir A0 //SharpIR is connected to A0
#define alcohol A1//Alcohol sensor is connected to A1
#define model 1080 //needed for sharpIR library


RF24 radio(7, 8); //initialize nrf
SharpIR sharp(ir, 25, 93, model);//initialize sharp IR
const byte address[6] = "00001";
char text[32] = "";
int distance = 0;
int drunkLevel = 0;
bool helmet = 0;
bool drunk = 0;
char msg[20];
void setup() {
  Serial.begin(115200);
  pinMode (ir, INPUT);
  pinMode (alcohol, INPUT);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

}
/*
green-12
red-11
orange-8
violet-7
yellow-3.3
ash-gnd
blue-13
*/
void loop() {

  distance = sharp.distance();
  Serial.print("IR distance : ");
  Serial.print(distance);
  Serial.print(" ");
  if ((distance>80)||(distance < 14)||(distance<0))helmet = 1;
  else helmet = 0;
  drunkLevel = analogRead(alcohol);
  Serial.print("Alcohol Level : ");
  Serial.println(drunkLevel);
  //Serial.println(drunkLevel);
  if (drunkLevel > 220)drunk = 1;
  else drunk = 0;

  if (helmet == 0 && drunk == 0) {
    sprintf(msg, "NONO","");
   
    radio.write(&msg, sizeof(msg));
  }
  else if (helmet == 0 && drunk == 1) {
   
    sprintf(msg, "NOYES","");
    radio.write(&msg, sizeof(msg));
  }
  else if (helmet == 1 && drunk == 0) {
    
     sprintf(msg, "YESNO","");
    radio.write(&msg, sizeof(msg));
  }
  else if (helmet == 1 && drunk == 1) {
   
     sprintf(msg, "YESYES","");
    radio.write(&msg, sizeof(msg));
  }

  
delay(500);

}

