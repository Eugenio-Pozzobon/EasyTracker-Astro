double STP_SPEED=1.89363;

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

#include "Wire.h"
#include "MPU6050_bdt.h"
#include "HMC5883L_bdt.h"

#ifdef BLUETOOTH
#include <SoftwareSerial.h>
SoftwareSerial mySerial(15, 16); // RX/TX
long unsigned int t_bt;
#define state_bt 2

#ifndef bt_hz
#define bt_hz 5
#endif

#endif

MPU6050 accelgyro;
HMC5883L mag;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;

#define LED_PIN 13
bool blinkState = false;

// fatores de correção determinados na calibração
int16_t mxMin, myMin, mzMin, mxMax, myMax, mzMax;

float xC = 0, yC = 0, zC = 0;
float heading = 0, heading_angle = 0;

#define mediaMovelArray 16
float reads[mediaMovelArray];

long unsigned int t_gy;

float denomX_A, denomX_B, denomX_C, denomX_T;
float denomY_A, denomY_B, denomY_C, denomY_T;
float denomZ_A, denomZ_B, denomZ_C, denomZ_T;
float angleX, angleY, angleZ;

float acelx, acely, acelz, rate_gyr_x, rate_gyr_y, rate_gyr_z, gyroXangle, gyroYangle, gyroZangle;
float AccXangle, AccYangle, AccZangle, CFangleX, CFangleY, CFangleZ;
float const_calib = 16071.82;
float const_gravid = 9.81;

unsigned long pT;

#include "stp.h"
// Define number of steps per rotation:
const int stepsPerRevolution = 4096 / 2;
Stepper myStepper = Stepper(stepsPerRevolution, 5, 11, 10, 12); //5 -> 9
boolean stepperState = false;

#define startbutton 2
#define stopbutton 3

#ifdef DEBUGTIMER
unsigned long steppertimer = 0, looptimer = 0;
#endif

void setup() {

  for (int i = 0; i < mediaMovelArray; i++) {
    reads[i] = 0;
  }
    pT = 0;

  BtSart();

  Serial.begin(115200);

  setupi2c();

  // configure Arduino pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(startbutton, INPUT_PULLUP);
  pinMode(stopbutton, INPUT_PULLUP);

  myStepper.setSpeed(STP_SPEED);
}

void loop() {
  if (!digitalRead(startbutton) && digitalRead(stopbutton)) {
    stepperState = false;
#ifdef DEBUGTIMER
    steppertimer = micros();
#endif
    myStepper.step(10);
#ifdef DEBUGTIMER
    Serial.print("stepper timer: ");  Serial.print(micros() - steppertimer);
    Serial.print("\t\tdata timer: ");   Serial.print(-looptimer + steppertimer);
    Serial.print("\t\tloop timer: ");   Serial.println(micros() - looptimer);
    //double a = 1000000 / float(micros() - looptimer);
    looptimer = micros();
#endif
  }
  else {
    stepperState = false;
  }
  
  
  if (((millis() - t_gy) > int(1000 / gy_hz)) && !stepperState) {

    t_gy = millis();

    accelgyro.readNormalizeAccel();
    accelgyro.readNormalizeGyro();

    ax = accelgyro.nax;
    ay = accelgyro.nay;
    az = accelgyro.naz;
    gx = accelgyro.ngx;
    gy = accelgyro.ngy;
    gz = accelgyro.ngz;

    mag.getHeading(&mx, &my, &mz);

    angleCalculation();
    compassCalculation();
    
    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);

    debugSensor();
  }

#ifdef BLUETOOTH
  sendBtData();
#endif

  if (Serial.available() > 0 ) {
    if (Serial.read() == 'c') {
      compassCalibration();
    }
  }
}
