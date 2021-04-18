//Código referente à inicialização da rede I2C e do MPU, junto com uma checagem. 
void setupi2c() {
  beginWire();

  Wire.begin();

  // initialize device
  Serial.println("Initializing I2C devices...");

  if (accelgyro.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G, MPU6050_NORMAL, WIRE_3400KhZ)) {
    Serial.println("\t \t MPU6050 connection successful");
    delay(100);
    accelgyro.setI2CMasterModeEnabled(false);
    accelgyro.setI2CBypassEnabled(true);
    accelgyro.setSleepEnabled(false);
    Serial.println("\t \t MPU6050 configurate successful");
  } else {
    Serial.println("\t \t *MPU6050 connection failed*");
  }

#ifdef MPU_FILTER
  accelgyro.setDHPFMode(MPU6050_DHPF_5HZ);
  accelgyro.setDLPFMode(MPU6050_DLPF_4);
#endif
  //checkSettings();

  mag.initialize();
  Serial.println(mag.testConnection() ? "\t \t HMC5883L connection successful" : "*HMC5883L connection failed*");

}

void checkSettings() {
#ifdef MPU_FILTER
    Serial.println("\t \t FILTRO DEFINED");
#endif

    Serial.print("\t \t * Sleep Mode:            ");
    Serial.println(accelgyro.getSleepEnabled() ? "Enabled" : "Disabled");

    Serial.print("\t \t * Clock Source:          ");
    switch (accelgyro.getClockSource()){
      case MPU6050_CLOCK_KEEP_RESET:     Serial.println("Stops the clock and keeps the timing generator in reset"); break;
      case MPU6050_CLOCK_EXTERNAL_19MHZ: Serial.println("PLL with external 19.2MHz reference"); break;
      case MPU6050_CLOCK_EXTERNAL_32KHZ: Serial.println("PLL with external 32.768kHz reference"); break;
      case MPU6050_CLOCK_PLL_ZGYRO:      Serial.println("PLL with Z axis gyroscope reference"); break;
      case MPU6050_CLOCK_PLL_YGYRO:      Serial.println("PLL with Y axis gyroscope reference"); break;
      case MPU6050_CLOCK_PLL_XGYRO:      Serial.println("PLL with X axis gyroscope reference"); break;
      case MPU6050_CLOCK_INTERNAL_8MHZ:  Serial.println("Internal 8MHz oscillator"); break;
    }

    Serial.print("\t \t * Accelerometer:         ");
    switch (accelgyro.getRange()){
      case MPU6050_RANGE_16G:            Serial.println("+/- 16 g"); break;
      case MPU6050_RANGE_8G:             Serial.println("+/- 8 g"); break;
      case MPU6050_RANGE_4G:             Serial.println("+/- 4 g"); break;
      case MPU6050_RANGE_2G:             Serial.println("+/- 2 g"); break;
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
      case MPU6050_SCALE_2000DPS:        Serial.println("2000 dps"); break;
      case MPU6050_SCALE_1000DPS:        Serial.println("1000 dps"); break;
      case MPU6050_SCALE_500DPS:         Serial.println("500 dps"); break;
      case MPU6050_SCALE_250DPS:         Serial.println("250 dps"); break;
    }

    Serial.print("\t \t * Gyroscope offsets: ");
    Serial.print(accelgyro.getGyroOffsetX());
    Serial.print(" / ");
    Serial.print(accelgyro.getGyroOffsetY());
    Serial.print(" / ");
    Serial.println(accelgyro.getGyroOffsetZ());
}

void beginWire() {
  Serial.println("Initializing I2C Bus...");
  int rtn = I2C_ClearBus(); // clear the I2C bus first before calling Wire.begin()
  if (rtn != 0) {
    Serial.println(("\t I2C bus error. Could not clear"));
    if (rtn == 1) {
      Serial.println(("\t SCL clock line held low"));
    } else if (rtn == 2) {
      Serial.println(("\t SCL clock line held low by slave clock stretch"));
    } else if (rtn == 3) {
      Serial.println(("\t SDA data line held low"));
    }
  } else {
    // bus clear
    // re-enable Wire
    // now can start Wire Arduino master
    Wire.begin();
    Serial.println("\t I2C bus clear. Init Sucesful");
    scanI2C();
  }
}

//Programa para limpeza da rede I2C. Adicionalmente faz um scanner, printado no começo do código. NÃO MEXER!
int I2C_ClearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

  pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(SCL, INPUT_PULLUP);

  delay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master.
    Serial.println("\t I2C bus error. Could not clear SCL clock line held low");
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
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
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      Serial.println("I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec");
      return 2;
    }
    SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    Serial.println(" I2C bus error. Could not clear. SDA data line held low");
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

void scanI2C() {
  byte error, address;
  int nDevices;

  Serial.println("Scanning I2C Bus...  ");

  nDevices = 0;
  for (address = 1; address < 127; address++ ){
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0){
      Serial.print("\t I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);

      nDevices++;
    }
    else if (error == 4){
      Serial.print("\t Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0){
    Serial.println("\t No I2C devices found");
  }else{
    Serial.println("\t Done scanner I2C\n");
  }
}

