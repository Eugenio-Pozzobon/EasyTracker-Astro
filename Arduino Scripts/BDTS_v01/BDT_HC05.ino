

void BtSart(){
  #ifdef BLUETOOTH
  mySerial.begin(9600);   //Serial Bluetooth
  pinMode(state_bt, INPUT); //Enable pin HC05
  #endif
}

void sendBtData(){
  #ifdef BLUETOOTH
  if ((((mySerial.available() && ((millis() - t_bt) > int(1000 / bt_hz))) || ((millis() - t_bt) > 1000) && (digitalRead(state_bt))) && !stepperState)) {
    mySerial.print(angleX); mySerial.print(",");
    mySerial.print(angleY); mySerial.print(",");
    mySerial.println(mediaMovel(reads));
    mySerial.flush();
    t_bt = millis();
#ifdef DEBUG
    Serial.println("Bluetooth Connected!");
#endif
  } else if (stepperState) {
    mySerial.print("Stepper on!\n");
  }
  #endif
}