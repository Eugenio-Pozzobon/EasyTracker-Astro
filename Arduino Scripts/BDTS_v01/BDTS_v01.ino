//test 1: speed -> 2.5 rpm
//initial angle -> 61.3, 02:47  | Ok
//*****   angle -> 61.6, 02:48  | Ok
//*****   angle -> 61.9, 02:49  | Not Ok
//*****   angle -> 62.2, 02:50  | Not Ok
//*****   angle -> 63.1, 02:53  | Not Ok
//*****   angle -> 64.0, 02:56  | Not Ok

//test 2: speed -> 2.083 rpm
//initial angle -> 64.4, 02:57  | ok
//        angle -> 64.9, 02:59  | ok
//*****   angle -> 65.4, 03:01  | ok
//*****   angle -> 67.7, 03:09  | Not Ok

//test 3: speed -> 1.89363 rpm
//*****   angle -> 70.3,03 :19  | ok
//*****   angle -> 71.3, 03:23  | ok
//*****   angle -> 72.3, 03:27  | ok
//*****   angle -> 73.3, 03:31  | ok
//*****   angle -> 74.3, 03:35  | ok?
//*****   angle -> 75.3, 03:39  | ok?
//*****   angle -> 76.3, 03:43  | ok?
//*****   angle -> 77.3, 03:47  | ok
//*****   angle -> 78.3, 03:51  | ok
//*****   angle -> 79.4, 03:55  | Not ok
//*****   angle -> 80.4, 03:59  | Not ok
//--------------------------------------
//*****   angle -> 11.9, 04:08  | ok
//*****   angle -> 13.0, 04:12  | Not ok
//*****   angle -> 14.0, 04:16  | ok
//*****   angle -> 15.0, 04:20  | ok?
//*****   angle -> 16.0, 04:24  | ok?
//*****   angle -> 17.0, 04:28  | ok
//*****   angle -> 18.1, 04:32  | Not ok
//*****   angle -> 19.0, 04:36  | ok
//*****   angle -> 20.0, 04:40  | ok
//*****   angle -> 21.1, 04:44  | Not ok

#include "configuration.h"

//librarys includes
#include "Wire.h"
#include "MPU6050_bdt.h"
#include "HMC5883L_bdt.h"
#include "stp.h"

#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/wdt.h>

//bluetooth variables
SoftwareSerial mySerial(15, 16); // RX/TX
unsigned long t_bt;
unsigned long t_counter = 0;
#define state_bt 14

// acelerometer variables
MPU6050 accelgyro;
float angleX, angleY, angleZ;
unsigned long pT = 0;

// magnetometer variables
HMC5883L mag;
int16_t mx, my, mz;

// blinking led
#define LED_PIN 13
bool blinkState = false;

// Calibration factors
int16_t mxMin = EEPROM.read(0) << 8 | EEPROM.read(1);
int16_t myMin = EEPROM.read(2) << 8 | EEPROM.read(3);
int16_t mzMin = EEPROM.read(4) << 8 | EEPROM.read(5);
int16_t mxMax = EEPROM.read(6) << 8 | EEPROM.read(7);
int16_t myMax = EEPROM.read(8) << 8 | EEPROM.read(9);
int16_t mzMax = EEPROM.read(10) << 8 | EEPROM.read(11);

// Calculation variables
#define mediaMovelArray 8
float heading_angle = 0;
float reads[mediaMovelArray];

// timer variables
unsigned long t_gy, steppertimer = 0, looptimer = 0;

// stepper variables
#define startbutton 2
#define stopbutton 3
const int stepsPerRevolution = 4096 / 2;
boolean stepperState = false;
Stepper myStepper = Stepper(stepsPerRevolution, 5, 11, 10, 12); //5 -> 9

void setup() {
#ifdef WHATCHDOG
  wdt_enable(WDTO_1S);
#endif
  //Initializate Variables
  for (int i = 0; i < mediaMovelArray; i++) {
    reads[i] = 0;
  }

  //Initializate Serial
#ifdef DEBUG
  Serial.begin(115200);
  printCalibrationInfo();
#endif

  //Initializate Bluetooth
  bluetoothSart();

  //Initializate I2C Bus
  setupi2c();

  // configure Arduino pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(startbutton, INPUT_PULLUP);
  pinMode(stopbutton, INPUT_PULLUP);

  // Setup Stepper
  myStepper.setSpeed(STP_SPEED);
}

void loop() {
  
#ifdef WHATCHDOG
  wdt_reset();
#endif

  if (!digitalRead(startbutton) && digitalRead(stopbutton)) {
    stepperState = true;
#ifdef DEBUG
    steppertimer = micros();
    myStepper.step(10);
    Serial.print("stepper timer: ");  Serial.println(micros() - steppertimer);
#else
    myStepper.step(10);
#endif
  }
  else {
    stepperState = false;
  }


  if (((millis() - t_gy) > 10) && !stepperState) {
    t_gy = millis();

    accelgyro.readNormalizeAccel();
    accelgyro.readNormalizeGyro();

    mag.getHeading(&mx, &my, &mz);

    angleCalculation();
    compassCalculation();
    debugSensor();
  }

  serialCommunication();
  bluetoothCommunication();
  manageUserOption();


  // blink LED to indicate activity
  blinkState = !blinkState;
  digitalWrite(LED_PIN, blinkState);
}

void serialCommunication() {
#ifdef DEBUG
  if (Serial.available()) {
    if (Serial.read() == 'c') {
      compassCalibration();
    }
  }
#endif
}

void manageUserOption() {
  if (mySerial.read() == 'c') {
    compassCalibration();
  }
}
