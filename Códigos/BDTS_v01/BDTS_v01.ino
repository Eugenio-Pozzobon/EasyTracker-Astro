#include "configuration.h"

#include "Wire.h"
#include "MPU6050.h"
#include "HMC5883L.h"

#ifdef BLUETOOTH
#include <SoftwareSerial.h>
SoftwareSerial mySerial(15, 16); // RX/TX
long unsigned int t_bt;
#define state_bt 2
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

float reads[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

// Include the Arduino Stepper.h library:
#include <Stepper.h>
// Define number of steps per rotation:
const int stepsPerRevolution = 4096/2;
Stepper myStepper = Stepper(stepsPerRevolution, 9, 11, 10, 12);
boolean stepperState = false;
#define startbutton 2
#define stopbutton 3

#include "TimerOne.h"

void setup() {

#ifdef BLUETOOTH
  mySerial.begin(9600);   //Serial Bluetooth
  pinMode(state_bt, INPUT); //Enable pin HC05
#endif

  Serial.begin(500000);

  Wire.begin();
  accelgyro.setI2CMasterModeEnabled(false);
  accelgyro.setI2CBypassEnabled(true) ;
  accelgyro.setSleepEnabled(false);

  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  mag.initialize();
  Serial.println(mag.testConnection() ? "HMC5883L connection successful" : "HMC5883L connection failed");

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // configure Arduino pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(startbutton, INPUT_PULLUP);
  pinMode(stopbutton, INPUT_PULLUP);

  myStepper.setSpeed(2.5);

  pT = 0;
  
  attachInterrupt(digitalPinToInterrupt(stopbutton), stopbt, CHANGE);
}


void stopbt(){
}

unsigned long steppertimer = 0, looptimer = 0;

void loop() {
  if (!digitalRead(startbutton) && digitalRead(startbutton)) {
    stepperState = false;
    steppertimer = micros();
    myStepper.step(10);
#ifdef DEBUGTIMER
    Serial.print("stepper timer: "); Serial.print(micros() - steppertimer);
    Serial.print("  data timer: "); Serial.print(-looptimer + steppertimer);
    double a = 1000000/float(micros() - looptimer);
    Serial.print("  loop timer: "); Serial.println(micros() - looptimer);
#endif
    looptimer = micros();
  }
  else {
    stepperState = false;
  }
  if (((millis() - t_gy) > int(1000 / gy_hz)) && !stepperState) {

    t_gy = millis();

    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    mag.getHeading(&mx, &my, &mz);

    angleCalculation();
    compassCalculation();

#ifdef DEBUGSENSOR
    Serial.print("Angulo em graus: X: ");
    Serial.print(angleX);
    Serial.print("\tY: ");
    Serial.print(angleY);
    Serial.print("\tZ: ");
    Serial.print(angleZ);

    Serial.print("\t");
    Serial.print(" ||  mag:  ");
    Serial.print(mx); Serial.print("\t");
    Serial.print(my); Serial.print("\t");
    Serial.print(mz); Serial.print("\t");

    Serial.print("heading: ");
    Serial.println(mediaMovel(reads));
#endif
    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
  }

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

  if(Serial.available() > 0 ){
    if(Serial.read() == 'c'){
      compassCalibration();
    }
  }

}
