

void initializeDevicesI2c() {

#ifdef DEBUG
  Serial.println("Initializing I2C devices...");
#endif


  wdt_reset();
  if (accelgyro.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G, MPU6050_NORMAL, WIRE_400kHz)) {
#ifdef DEBUG
    Serial.print("\t MPU6050 connection successful \n");
#endif
    delay(10);
    accelgyro.setI2CMasterModeEnabled(false);
    accelgyro.setI2CBypassEnabled(true);
    accelgyro.setSleepEnabled(false);
    //accelgyro.setDHPFMode(MPU6050_DHPF_5HZ);
    accelgyro.setDLPFMode(MPU6050_DLPF_6);


#ifdef DEBUG
    Serial.print("\t MPU6050 configurate successful \n");
#endif

  } else {
#ifdef DEBUG
    Serial.print("\t *MPU6050 connection failed* \n");
#endif
  }

  mag.initialize();
#ifdef DEBUG
  Serial.print(mag.testConnection() ? "\t HMC5883L connection successful\n" : "*HMC5883L connection failed*\n");
#endif
}

/*
   Check I2C Lines, start bus and get device list
*/
void beginI2cBus() {

  wdt_reset();

  int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
  if (rtn != 0) {
#ifdef DEBUG
    Serial.print("\t I2C bus error. Could not clear \n");
#endif
    if (rtn == 1) {
#ifdef DEBUG
      Serial.print("\t SCL clock line held low \n");
#endif
    } else if (rtn == 2) {
#ifdef DEBUG
      Serial.print("\t SCL clock line held low by slave clock stretch \n");
#endif
    } else if (rtn == 3) {
#ifdef DEBUG
      Serial.print("\t SDA data line held low \n");
#endif
    }
  } else {
    // bus clear
    // re-enable Wire
    // now can start Wire Arduino master
    Wire.begin();
#ifdef DEBUG
    Serial.print("\t I2C bus clear. Init Sucesful\n");
    scanI2C();
#endif
  }
}


/*
   Clean I2C bus and return actual situation for start wire sucessfully
*/
int I2C_ClearBus() {

  wdt_reset();
  
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  unsigned long timerI2c = millis();
  while (millis() - timerI2c < 2500) {
    wdt_reset(); // Wait 2.5 secs. This is strictly only necessary on the first power
  }
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master.
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,


    wdt_reset();
    
    clockCount--;
    // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(SCL, INPUT); // release SCL LOW
    pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.

      wdt_reset();

      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2;
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(SDA, INPUT); // remove pullup.
  pinMode(SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(SDA, INPUT); // remove output low
  pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(SCL, INPUT);
  return 0; // all ok
}

/*
   Scan i2c bus with a for() loking for adress in all 127 spaces.
*/
void scanI2C() {
  byte error, address;
  int nDevices;

  Serial.print("Scanning I2C Bus...\n");

  nDevices = 0;
  for (address = 1; address < 127; address++ ) {

    wdt_reset();
    
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("\t I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);

      nDevices++;
    }
    else if (error == 4) {
      Serial.print("\t Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.print("\t No I2C devices found\n");
  } else {
    Serial.print("\t Done scanner I2C\n");
  }
}
