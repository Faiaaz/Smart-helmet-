
void initRf() { //initialize RF module
  rf.begin();
  rf.openReadingPipe(0, addressOfRf);
  rf.setPALevel(RF24_PA_MIN);
  rf.startListening();

}
void initilize() { //initialize receiver circuit

  initRf();


  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  oled("INITIALIZING         MODULES", 1);
  initGSM();
  loading();
}
void initGSM() { //initialize sim800L

  digitalWrite(GSM_RESET, 0);
  delay(4000);
  digitalWrite(GSM_RESET, 1);
}
void oled(char * inn, int size_txt) { //function to show data into OLED screen

  display.display();
  display.setFont( );
  display.setTextSize(size_txt);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(inn);
  display.display();

}
void sendSms() { //function for sending SMS if accident occures
  validNetwork();
  display.clearDisplay();
  for (int i = 5; i > 0; i--) { //count down to stop sending sms if false trigger happens
    sprintf(displayBuffer, "\n     %d", i);
    display.clearDisplay();
    oled("SENDING EMERGENCY SMS          IN", 1);
    oled(displayBuffer, 2);
    delay(1000);
  }

  if (netState == 1) { //if network available
    display.clearDisplay();
    oled("SENDING SMS", 1);
    Serial3.println((char)(26));//clear sim800L sms buffer if any previous sms was pending
    if  (sendATcommand("AT+CMGF=1", "OK", "i matter", "me too", 10000)) { // initialize SMS service
      sendATcommand("AT+CSMP=17,167,0,0", "OK", "i matter", "me too", 1000); //initializing SMS parameters
      Serial3.print("AT+CMGS=\""); //start writing SMS
      Serial3.print(recepient); //writing receipient number
      delay(1000);
      if (sendATcommand("\"", ">", "i matter", "me too", 10000)) {// writing SMS body
        if (isDriving) {
          Serial3.print(message1); 
          if (coordinateAvailable) { //only if co-ordinates are available,send co-ordinates
            Serial3.print(message2);
            Serial3.print(latBuffer);
            Serial3.print(",");
            Serial3.print(lngBuffer);
          }
        }
        
//        else{
//          Serial3.print(message3);
//            if (coordinateAvailable) {
//            Serial3.print(message2);
//            Serial3.print(latBuffer);
//            Serial3.print(",");
//            Serial3.print(lngBuffer);
//          }
//          }

        Serial3.println((char)(26));//end SMS
        if (sendATcommand("", "+CMGS:", "i_matter", "me_too", 30000)) { //wait for SMS send response 

          display.clearDisplay();
          isAccident=0;
          oled("SMS SENT", 2);
          delay(2000);
          display.clearDisplay();
          oled("PLEASE TURN OFF THE \nIGNITION", 1);
          while(isIgnition);
          
        }
        else  { //if sending SMS was not successful
          display.clearDisplay();
          oled("SMS FAILED", 2);
          delay(1000);
        }
      }
    }


  }
  else { //if no network detected
    display.clearDisplay();
    oled("CAN NOT DETECT CELLULAR NETWORK", 1);
  }
}

//this function is a non-blocking delay which can detect if gps signal is available or accident has occured
static void smartDelay(unsigned long ms) 
{
  unsigned long start = millis();
  do
  { if (!digitalRead(accident))isAccident = 1;
    while (Serial1.available())
      gps.encode(Serial1.read());
      checkBikerCondition();
  } while (millis() - start < ms);
}

void prepareGpsValues() { //read latitude,longitude and save them into latBuffer,lngBuffer respectively 

  float latValue = gps.location.lat();
  float lngValue = gps.location.lng();
  const char *tmpSign = (latValue < 0) ? "-" : ""; //handles negative latitude or longitude
  float tmpVal = (latValue  < 0) ? -latValue : latValue; //takes only positive value

  int tmpInt1 = tmpVal;                  // Get the integer (678).
  float tmpFrac = tmpVal - tmpInt1;      // Get fraction (0.0123).
  uint32_t tmpInt2 = trunc(tmpFrac * 100000);  // Turn into integer (123).

 

  sprintf (latBuffer, "%s%d.%ld", tmpSign, tmpInt1, tmpInt2); //store three parts (sign,before decimal point,after decimal point) into latBuffer
  tmpSign = (lngValue < 0) ? "-" : "";//same thing for longitude
  tmpVal = (lngValue  < 0) ? -lngValue : lngValue;

  tmpInt1 = tmpVal;                  // Get the integer (678).
  tmpFrac = tmpVal - tmpInt1;      // Get fraction (0.0123).
  tmpInt2 = trunc(tmpFrac * 1000000);  // Turn into integer (123).

  // Print as parts, note that you need 0-padding for fractional bit.

  sprintf (lngBuffer, "%s%d.%ld", tmpSign, tmpInt1, tmpInt2); //store three parts (sign,before decimal point,after decimal point) into lngBuffer
  Serial.print(latBuffer);
  Serial.print("-");
  Serial.println(lngBuffer);

}


int checkBikerCondition() {
  if (rf.available()) { //if data is available
    while (rf.available()) {
      rf.read(&text, sizeof(text)); // read the data and store into text buffer
    }
    //  Serial.println(text);


    if (strstr(text, "NONO") != NULL) { //parse text buffer if "NONO" is received
      Serial.println("No helmet and sober");
      isDataReceived = 1; //set data received flag to 1
      safeStart = 0;      //set helmet flag to 0
      isDrunk = 0;        //set drunk flag to 0
    }
    if (strstr(text, "NOYES") != NULL) {
      Serial.println("No helmet and drunk");
      isDataReceived = 1;
      safeStart = 0;
      isDrunk = 1;
    }
    if (strstr(text, "YESNO") != NULL) {
      Serial.println("Wore helmet and sober");
      isDataReceived = 1;
      safeStart = 1;
      isDrunk = 0;
    }
    if (strstr(text, "YESYES") != NULL) {
      Serial.println("Wore helmet but drunk");
      isDataReceived = 1;
      safeStart = 1;
      isDrunk = 1;
    }

  }

}


void loading() {   //wait for all modules to start up

  for (int load = 0; load < 101; load++) {


    delay(50);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(11, 3);

    display.print("Initializing...");
    display.drawRoundRect(12, 15, 100, 3, 0, 1);
    display.fillRoundRect(12, 15, load, 3, 0, 1);
    display.setFont( );

    display.setCursor(55, 20);
    sprintf(displayBuffer, "%d", load);
    display.print(displayBuffer);
    display.print("%");
    display.display();
  }


}
