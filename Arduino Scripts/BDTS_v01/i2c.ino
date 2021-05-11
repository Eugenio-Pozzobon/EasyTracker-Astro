//Código referente à inicialização da rede I2C e do MPU, junto com uma checagem.
void setupi2c() {
#ifdef WHATCHDOG
  wdt_reset();
#endif
  beginWire();
#ifdef DEBUG
  Serial.println("Initializing I2C devices...");
#endif

  // initialize device

#ifdef WHATCHDOG
  wdt_reset();
#endif
  if (accelgyro.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G, MPU6050_NORMAL, WIRE_3400KhZ)) {
#ifdef DEBUG
    Serial.print("\t MPU6050 connection successful \n");
#endif
    delay(10);
    accelgyro.setI2CMasterModeEnabled(false);
    accelgyro.setI2CBypassEnabled(true);
    accelgyro.setSleepEnabled(false);
    accelgyro.setDHPFMode(MPU6050_DHPF_5HZ);
    accelgyro.setDLPFMode(MPU6050_DLPF_4);


#ifdef DEBUG
    Serial.print("\t MPU6050 configurate successful \n"); //checkSettings(); //it requires alot of variable memory!
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
   Check acelerometer settings
   Not recomended because it requires alot of memory data
*/
void checkSettings() {

#ifdef WHATCHDOG
  wdt_reset();
#endif
  Serial.print("\t *Check Settings \n");

  Serial.print("\t \t * Sleep Mode:            ");
  Serial.println(accelgyro.getSleepEnabled() ? "Enabled" : "Disabled");

  Serial.print("\t \t * Clock Source:          ");
  switch (accelgyro.getClockSource()) {
    case MPU6050_CLOCK_KEEP_RESET:     Serial.print("Stops the clock and keeps the timing generator in reset\n"); break;
    case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.print("PLL with external 19.2MHz reference\n"); break;
    case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.print("PLL with external 32.768kHz reference\n"); break;
    case MPU6050_CLOCK_PLL_ZGYRO:      Serial.print("PLL with Z axis gyroscope reference\n"); break;
    case MPU6050_CLOCK_PLL_YGYRO:      Serial.print("PLL with Y axis gyroscope reference\n"); break;
    case MPU6050_CLOCK_PLL_XGYRO:      Serial.print("PLL with X axis gyroscope reference\n"); break;
    case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.print("Internal 8MHz oscillator\n"); break;
  }

  Serial.print("\t \t * Accelerometer:         ");
  switch (accelgyro.getRange()) {
    case MPU6050_RANGE_16G:            Serial.print("+/- 16 g\n"); break;
    case MPU6050_RANGE_8G:             Serial.print("+/- 8 g\n"); break;
    case MPU6050_RANGE_4G:             Serial.print("+/- 4 g\n"); break;
    case MPU6050_RANGE_2G:             Serial.print("+/- 2 g\n"); break;
  }

  Serial.print("\t \t * Accelerometer offsets: ");
  Serial.print(accelgyro.getAccelOffsetX());
  Serial.print(" / ");
  Serial.print(accelgyro.getAccelOffsetY());
  Serial.print(" / ");
  Serial.println(accelgyro.getAccelOffsetZ());

  Serial.print("\t \t * Gyroscope:         ");
  switch (accelgyro.getScale())
  {
    case MPU6050_SCALE_2000DPS:        Serial.print("2000 dps\n"); break;
    case MPU6050_SCALE_1000DPS:        Serial.print("1000 dps\n"); break;
    case MPU6050_SCALE_500DPS:         Serial.print("500 dps\n"); break;
    case MPU6050_SCALE_250DPS:         Serial.print("250 dps\n"); break;
  }

  Serial.print("\t \t * Gyroscope offsets: ");
  Serial.print(accelgyro.getGyroOffsetX());
  Serial.print(" / ");
  Serial.print(accelgyro.getGyroOffsetY());
  Serial.print(" / ");
  Serial.println(accelgyro.getGyroOffsetZ());
}

/*
   Check I2C Lines, start bus and get device list
*/
void beginWire() {

#ifdef WHATCHDOG
  wdt_reset();
#endif
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

#ifdef WHATCHDOG
  wdt_reset();
#endif
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  unsigned long timerI2c = millis();
  while (millis() - timerI2c < 2500) {
#ifdef WHATCHDOG
    wdt_reset();
#endif// Wait 2.5 secs. This is strictly only necessary on the first power
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

#ifdef WHATCHDOG
    wdt_reset();
#endif
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
#ifdef WHATCHDOG
      wdt_reset();
#endif
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
#ifdef WHATCHDOG
    wdt_reset();
#endif
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
