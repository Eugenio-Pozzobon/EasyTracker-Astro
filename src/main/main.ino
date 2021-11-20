

#include "configuration.h"

// Librarys includes
#include "Wire.h"
#include "MPU6050_bdt.h"
#include "HMC5883L_bdt.h"
#include "stp.h"
//#include <Stepper.h>

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/wdt.h>

// Bluetooth variables
SoftwareSerial bluetoothSerial(16, 15); // RX/TX
unsigned long timerBluetoothRestartMessages = 0;
unsigned long timerBluetoothRecieveMessages = 0;
unsigned long timerBluetoothSendMessages = 0;
const byte bluetoothStatePin = 13;

//User Input Variables
char availableUserOption = '0';
bool manualDeactivation = true, remoteDeactivation = false;
bool manualActivation = false, lastManualActivation = false;
bool remoteActivation = false;

// Acelerometer variables
MPU6050 accelgyro;
float angleX, angleY, angleZ;
unsigned long gyroIntegrateTimer = 0; //Timer to interate Gyroscope axis and get Gyro part of Pitch and Roll angle.
const float accelGyroRelation = 0.5; //How mucth Gyro and Accel is consider for interate Pitch and Roll. 0 means that only accel will be used.

// Magnetometer variables
HMC5883L mag;
int mx, my, mz;
float mxCalibrated, myCalibrated, mzCalibrated;

// Calibration factors
int mxMin = EEPROM.read(0) << 8 | EEPROM.read(1);
int myMin = EEPROM.read(2) << 8 | EEPROM.read(3);
int mzMin = EEPROM.read(4) << 8 | EEPROM.read(5);
int mxMax = EEPROM.read(6) << 8 | EEPROM.read(7);
int myMax = EEPROM.read(8) << 8 | EEPROM.read(9);
int mzMax = EEPROM.read(10) << 8 | EEPROM.read(11);

// Calculation variables
const byte mediaMovelArray = 8;
float compassAngle = 0;
float reads[mediaMovelArray];

// Timer variables
unsigned long timerMpuData, timerStepperMicros = 0, timerLoop = 0;

// Stepper variables
boolean stepperState = false;
const int startButtonPin = 14;
const int stopButtonPin = 2;
const int stopButtonPin_sec = 3;
const int stepsPerRevolution = 4096 / 2;
HalfStepper myStepper = HalfStepper(stepsPerRevolution, 12, 10, 11, 9);

void setup() {
  //Initializate Whatchdog
  wdt_enable(WDTO_8S);

  // Initializate Variables
  for (int i = 0; i < mediaMovelArray; i++) {
    reads[i] = 0;
  }

  // Initializate Serial
#ifdef DEBUG
  Serial.begin(115200);
  printCalibrationInfo();
#endif

  // Initializate Bluetooth
  bluetoothSart();

  // Initializate I2C Bus
  beginI2cBus();
  initializeDevicesI2c();

  // Initializate Arduino pins
  //  pinMode(ledPin, OUTPUT);
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(stopButtonPin, INPUT_PULLUP);
  pinMode(stopButtonPin_sec, INPUT_PULLUP);

  // Setup Stepper
  myStepper.setSpeed(STP_SPEED);
}

void loop() {

  wdt_reset();

  stepperState = checkStepperCondition();
  if (stepperState == true) {
    myStepper.step(runningSteps); //runs Stepper motor and elevate the plataform
    if (bluetoothSerial.available() > 0) {
      if (manualActivation && !remoteActivation) {
        bluetoothSerial.print("s,s\n");
      }
      availableUserOption = bluetoothSerial.read(); //flush buffer
    }
  } else {
    if (bluetoothSerial.available() > 0) {
      if (remoteActivation) {
        bluetoothSerial.print("n,n\n");
        remoteActivation = false;
      }
    }
  }

  if (stepperState == false) {
    getData();
    serialCommunication();
    bluetoothCommunication();
  }

  manageUserOption();

#ifdef DEBUG
//  Serial.print("manualActivation "); Serial.println(manualActivation);
//  Serial.print("manualDeactivation "); Serial.println(manualDeactivation);
//  Serial.print("remoteActivation "); Serial.println(remoteActivation);
//  Serial.print("remoteDeactivation "); Serial.println(remoteDeactivation); Serial.println();
#endif
}


bool checkStepperCondition() {
  int nullvar = analogRead(startButtonPin); //não tem função nenhuma, não serve pra nada, porém o código só funciona se estiver aqui!
  if (millis() > 5000) {// dont get the starting set of pullup as a true value
    if (!manualActivation && !digitalRead(startButtonPin)) {
      manualActivation = true;
      manualDeactivation = false;
      remoteDeactivation = false;
    }
    if (manualActivation && digitalRead(startButtonPin)) {
      manualActivation = false;
      manualDeactivation = true;
      remoteActivation = false;
    }
  }
  bool stopState = false;//(!digitalRead(stopButtonPin) || !digitalRead(stopButtonPin_sec));
  if(stopState){
    manualActivation = false;
    remoteActivation = false;
  }
  return ((manualActivation || remoteActivation) && (!manualDeactivation && !remoteDeactivation && !stopState));
}

bool checkBluetoothCondition() {
  return (!digitalRead(bluetoothStatePin));
}

void serialCommunication() {
#ifdef DEBUG
  while (Serial.available() > 0) {
    availableUserOption = Serial.read();
  }
#endif
}


/*
   @brief
   @note
   @param
   @return
*/
void manageUserOption() {
  if (availableUserOption == 'c') {
    compassCalibration();
  }
  if (availableUserOption == 's') {
    remoteActivation = true;
    remoteDeactivation = false;
    manualDeactivation = false;
    bluetoothSerial.print("s,s\n"); //while writebuffer isnt clean in android, send the buffer that trigger an internal clean
  }
  if (availableUserOption == 'n') {
    remoteDeactivation = true;
    remoteActivation = false;
    bluetoothSerial.print("n,n\n"); //while writebuffer isnt clean in android, send the buffer that trigger an internal clean
  }
  if (availableUserOption == 'r') {
    ressetCalibration();
  }
  if (availableUserOption == '1') {
    remoteActivation = true;
    remoteDeactivation = false;
  }
  availableUserOption = '0';
}
