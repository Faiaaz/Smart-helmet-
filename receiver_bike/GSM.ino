//function for sending command to sim800L and parse the response from it

int8_t sendATcommand(char * ATcommand, char * expected_answer1, char * expected_answer2, char * expected_answer3, unsigned int timeout) { 
  uint8_t x = 0, answer = 0;

  unsigned long previous;

  memset(response, '\0', 100); // Initialize the string

  delay(100);

  while (Serial3.available() > 0) Serial3.read(); // Clean the input buffer

//Serial.println(ATcommand); // Send the AT command

  Serial3.println(ATcommand); // Send the AT command
  Serial.println(ATcommand);
  
  x = 0;
  delay(50);
  previous = millis();

  // this loop waits for the answer
  do {
    // if there are data in the UART input buffer, reads it and checks for the asnwer
    if (Serial3.available() != 0) {
      response[x] = Serial3.read();
      Serial.print(response[x]);
      x++;
      // check if the desired answer 1 is in the response of the module
      if (strstr(response, expected_answer1) != NULL) {
        
        answer = 1;
      }
      // check if the desired answer 2 is in the response of the module
      if (strstr(response, expected_answer2) != NULL) {
        answer = 2;
      }
      if (strstr(response, expected_answer3) != NULL) {
        answer = 3;
      }
    }
    // Waits for the asnwer with time out
  } while ((answer == 0) && ((millis() - previous) < timeout));
  // if((millis() - previous) == timeout){
  //   Serial.println("TIME OUT");
  //
  // }
  return answer;
}
//check if the sim800L has been connected to the cellular network,if not try to connect
bool validNetwork() { 
  if (netState == 0) {
    k = 0;
    display.clearDisplay();
    oled("CONNECTING TO THE    NETWORK...", 1);
    while (sendATcommand("AT+CREG?", "+CREG: 0,1", "+CREG: 0,5", "nai", 2000) == 0 && k < 5) {
      k++;
    };

    if (k == 5) {
      netState = 0;
      display.clearDisplay();
      oled("NETWORK ERROR", 1);
      delay(1000);
      return 0;
    }
    else netState=1;;
  }
  else netState=1;;

}

