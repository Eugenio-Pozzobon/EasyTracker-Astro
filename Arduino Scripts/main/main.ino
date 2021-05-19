#include "configuration.h"

// Librarys includes
#include "Wire.h"
#include "MPU6050_bdt.h"
#include "HMC5883L_bdt.h"
#include "stp.h"

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/wdt.h>

// Bluetooth variables
SoftwareSerial bluetoothSerial(15, 16); // RX/TX
unsigned long timerBluetoothRestartMessages = 0;
unsigned long timerBluetoothRecieveMessages = 0;
unsigned long timerBluetoothSendMessages = 0;
const byte bluetoothStatePin = 14;

//User Input Variables
byte availableUserOption = 0;
bool userSelectedStart = false;

// Acelerometer variables
MPU6050 accelgyro;
float angleX, angleY, angleZ;
unsigned long gyroIntegrateTimer = 0; //Timer to interate Gyroscope axis and get Gyro part of Pitch and Roll angle.
const float accelGyroRelation = 0.5; //How mucth Gyro and Accel is consider for interate Pitch and Roll. 0 means that only accel will be used.

// Magnetometer variables
HMC5883L mag;
int mx, my, mz;
float mxCalibrated, myCalibrated, mzCalibrated;

// Blinking led
const byte ledPin = 13;
bool blinkState = false;

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
byte runningSteps = 10;
const int startButtonPin = 2;
const int stopButtonPin = 3;
const int stepsPerRevolution = 4096 / 2;
Stepper myStepper = Stepper(stepsPerRevolution, 5, 11, 10, 12); //5 -> 9

void setup() {
  //Initializate Whatchdog
  wdt_enable(WDTO_1S);

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
  pinMode(ledPin, OUTPUT);
  pinMode(startButtonPin, INPUT_PULLUP);
  pinMode(stopButtonPin, INPUT_PULLUP);

  // Setup Stepper
  myStepper.setSpeed(STP_SPEED);
}

void loop() {

  wdt_reset();

  stepperState = checkStepperCondition();
  if (stepperState == true) {
    myStepper.step(runningSteps); //runs Stepper motor and elevate the plataform
  }

  if (stepperState == false) {
    getData();
    serialCommunication();
    bluetoothCommunication();
    manageUserOption();
  }

  // blink LED to indicate activity
  blinkState = !blinkState;
  digitalWrite(ledPin, blinkState);
}


bool checkStepperCondition() {
  return ((!digitalRead(startButtonPin) || userSelectedStart) && digitalRead(stopButtonPin));
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
    //userSelectedStart = true;
  }

  if (availableUserOption == 'r') {
    ressetCalibration();
  }
  availableUserOption = '0';
}
