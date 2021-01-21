#include "configuration.h"

#include "Wire.h"
#include "MPU6050.h"
#include "HMC5883L.h"

#ifdef BLUETOOTH
#include <SoftwareSerial.h>
SoftwareSerial mySerial(3, 4); // RX/TX
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

long unsigned int t_gy;

float denomX_A, denomX_B, denomX_C, denomX_T;
float denomY_A, denomY_B, denomY_C, denomY_T;
float denomZ_A, denomZ_B, denomZ_C, denomZ_T;
float angleX, angleY, angleZ;

// Include the Arduino Stepper.h library:
#include <Stepper.h>
// Define number of steps per rotation:
const int stepsPerRevolution = 2048;
Stepper myStepper = Stepper(stepsPerRevolution, 6, 10, 7, 11);
boolean stepperState = false;
#define startbutton 12

void setup() {

#ifdef BLUETOOTH
  mySerial.begin(9600);   //Serial Bluetooth
  pinMode(state_bt, INPUT); //Enable pin HC05
#endif

  Serial.begin(115200);

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
  compassCalibration();
  myStepper.setSpeed(10);
  myStepper.step(10);
}

void loop() {
  if (!digitalRead(startbutton)) {
    stepperState = true;
    myStepper.step(10);
  }
  else {
    stepperState = false;
  }
  if (((millis() - t_gy) > int(1000 / gy_hz)) && !stepperState) {

    t_gy = millis();

    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    mag.getHeading(&mx, &my, &mz);

    if (mxMax < mx) {
      mxMax = mx;
    }
    if (mxMin > mx) {
      mxMin = mx;
    }
    if (myMax < my) {
      myMax = my;
    }
    if (myMin > my) {
      myMin = my;
    }
    if (mzMax < mz) {
      mzMax = mz;
    }
    if (mzMin > mz) {
      mzMin = mz;
    }

    angleCalculation();
    compassCalculation();

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
    Serial.println(heading_angle);

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
  } else if (stepperState) {
    Serial.print("Stepper on!\n");
  }

#ifdef BLUETOOTH
  if ((((mySerial.available() && ((millis() - t_bt) > int(1000 / bt_hz))) || ((millis() - t_bt) > 1000) && (digitalRead(state_bt))) && !stepperState) {
    mySerial.print(angleX); mySerial.print(",");
    mySerial.print(angleY); mySerial.print(",");
    mySerial.println(heading_angle);
    mySerial.flush();
    t_bt = millis();
    Serial.println("Bluetooth Connected!");
  } else if (stepperState) {
#ifdef BLUETOOTH
    mySerial.print("Stepper on!\n");
#endif
  }
#endif
}
