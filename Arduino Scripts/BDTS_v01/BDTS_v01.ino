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
Stepper myStepper = Stepper(stepsPerRevolution, 9, 11, 10, 12);
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
  sendBtData()
#endif

  if (Serial.available() > 0 ) {
    if (Serial.read() == 'c') {
      compassCalibration();
    }
  }
}
