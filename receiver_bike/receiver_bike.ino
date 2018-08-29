//Libraries

#define VERSION "9JAN18"
#include<SPI.h>
#include "RF24.h"
#include <Adafruit_GFX.h>  //OLED            
#include <Adafruit_SSD1306.h>  //OLED
#include "Fonts/FreeSerifItalic9pt7b.h"

#include <TinyGPS++.h>

//Interfacing defines
#define ignition 3
#define accident 2
#define GSM_RESET 5
#define OLED_RESET 4  //OLED
#define isIgnition !digitalRead(ignition)
//Conditional defines

//Object Declaration

RF24 rf(8, 53);
Adafruit_SSD1306 display(OLED_RESET);  //OLED
TinyGPSPlus gps;
// The serial connection to the GPS device

//Variable Declaration

static const uint32_t GPSBaud = 9600;
char displayBuffer[20];
char latBuffer[15];
char lngBuffer[15];
char response[100];
uint8_t dataDump[26] = {0};
const byte addressOfRf[6] = "00001";
char text[32] = "";
int k = 0;
//Flags
bool netState = 0;
bool coordinateAvailable = 0;
char recepient[14] = "8801676736361";
char message1[30] = "Accident has happened";
char message2[50] = " at location https://www.google.com/maps/place/";
char message3[30] = "Your bike has fallen";
char locationCoordinates[30] = {0};
bool isAccident = 0;
bool safeStart = 0;
bool isDrunk = 0;
bool isDriving = 0;
bool isDataReceived = 0;
void setup() {
  Serial.begin(115200);
  Serial1.begin(GPSBaud);
  Serial3.begin(115200);
  pinMode(13, OUTPUT);
  pinMode(ignition, INPUT);

  pinMode(accident, INPUT);
  pinMode(GSM_RESET, OUTPUT);
  digitalWrite(ignition, HIGH);
  initilize();


  validNetwork();
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();

  checkBikerCondition();              //check if any data is received from transmitter and parse the data

  if ( gps.location.isValid()) {
    coordinateAvailable = 1; 
    prepareGpsValues();              //read latitude,longitude and save them into latBuffer,lngBuffer respectively 
  }
  if (!isDataReceived) {             //if no valid data received from transmitter since start up 
    oled("TURN ON THE HELMET   AND WEAR IT", 1);

  }


  
  if (isIgnition && !isDriving && !isAccident) {  //if rider has push ignition button, was not driving and no accident happened
    if (safeStart && !isDrunk) {                  //if the rider has worn helmet and not drunk
      oled("\n\nSTARTING IGNITION", 1);
      digitalWrite(13, HIGH);
      isDriving = 1;
    }

    if (safeStart && isDrunk && isDataReceived) { //if the rider has worn helmet and  drunk
      oled("\n\nYOU ARE DRUNK; NOT SAFE TO DRIVE", 1);

    }

    if (!safeStart && !isDrunk && isDataReceived) {//if the rider has not worn helmet and not drunk
      oled("\n\nPLEASE WEAR HELMET", 1);

    }

    if (!safeStart && isDrunk && isDataReceived) {////if the rider has not worn helmet and  drunk
      oled("\n\nBE SOBER AND\nWEAR HELMET ", 1);

    }
  }




  
  if (isDriving && (!safeStart || isDrunk) ) { //if while driving no helmet or alcohol is detected
    display.clearDisplay();
    oled("IGNITION OFF", 1);
    digitalWrite(13, LOW);
    isDriving = 0;
    

  }


  
  if (isIgnition && isDriving && isDataReceived) { //rider is driving in proper condition
    display.clearDisplay();
    oled("DRIVE SAFE", 2);

  }


  
  if (!isIgnition && isDriving) {  //rider was driving but now turning the ignition off i.e,turn off ignition button
    display.clearDisplay();
    oled("IGNITION OFF", 1);
    digitalWrite(13, LOW);
    isDriving = 0;
  }


  
  if (!isIgnition && !isDriving && isDataReceived) { //not driving, ignition off and helmet connected 
    display.clearDisplay();
    oled(" WELCOME", 2);

  }






  if (((!digitalRead(accident) || isAccident) && isDriving ) ) { //Accident occured while driving
    display.clearDisplay();
    oled("IGNITION OFF", 1);
    digitalWrite(13, LOW);
    sendSms();
    isDriving = 0;

    delay(1000);

  }


  

  if (((!digitalRead(accident) || isAccident) && !isDriving ) ) { //if accident is triggered while not driving,discard the accident flag 
    
    isAccident=0;

  }



  smartDelay(1000);


}
