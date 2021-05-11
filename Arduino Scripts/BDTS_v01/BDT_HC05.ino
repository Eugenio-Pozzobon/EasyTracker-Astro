/*
   Initializate HC05 Serial Comunication
*/
void bluetoothSart() {
#ifdef WHATCHDOG
  wdt_reset();
#endif
  mySerial.begin(9600);   //Serial Bluetooth
  pinMode(state_bt, INPUT); //Enable pin HC05
}

/*
   Manages Bluetooth Comunication
*/
void bluetoothCommunication() {
  if (!stepperState && digitalRead(state_bt)) {

    if (mySerial.available() || ((millis() - t_counter) > 30) ) {
      if (mySerial.available()) {
        t_bt = millis();
        char recieve = mySerial.read(); //2ms for recieve a char
        #ifdef DEBUG
      Serial.print("\t Bluetooth Recieve");Serial.print(recieve);Serial.print("\n");
#endif
      }
      if ((millis() - t_bt) < 1000) { //it takes 16ms for send all angle buffer
        t_counter = millis();
        mySerial.print(int(angleX * 10)); mySerial.print(",");
        mySerial.print(int(angleY * 10)); mySerial.print(",");
        int MM = int(mediaMovel(reads) * 10);
        mySerial.print(MM); mySerial.print(",");
        mySerial.print(MM + int(angleX * 10) + int(angleY * 10));
        mySerial.print("\n");
#ifdef DEBUG
        Serial.print("\t Bluetooth Send \n");
#endif
      }
    }

    if ((millis() - t_bt) > 1000) {
      t_bt = millis();
      mySerial.print(int(angleX * 10)); mySerial.print(",");
      mySerial.print(int(angleY * 10)); mySerial.print(",");
      int MM = int(mediaMovel(reads) * 10);
      mySerial.print(MM); mySerial.print(",");
      mySerial.print(MM + int(angleX * 10) + int(angleY * 10));
      mySerial.print("\n");
#ifdef DEBUG
      Serial.print("\t Bluetooth Try \n");
#endif
    }
  } else if (stepperState) {

  }
}
