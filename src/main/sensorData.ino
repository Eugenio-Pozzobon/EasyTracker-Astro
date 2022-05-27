/*
sensorData.ino - file that process sensor data at EasyTracker

Version: 1.0.0
Copyright (C) 2022  Eugênio Pozzobon

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

void getData() {
  if (((millis() - timerMpuData) >= 0) && !stepperState) {

    timerMpuData = millis();
    //1ms to get all data with 400kHz and 5ms with 100kHz
    accelgyro.readNormalizeAccel();
    accelgyro.readNormalizeGyro();

    mag.getHeading(&mx, &my, &mz);


    angleCalculation();
    //compassCalculation();

    debugSensor();
  }
}

void angleCalculation() { //900us usando int_16t e 1300us usando float
  unsigned long cT = micros(); // contar tempo de loop
  unsigned long dT = cT - gyroIntegrateTimer; //Variable for measure angle with gps and make integration of values
  gyroIntegrateTimer = cT;

  // Gyro Calculations convert gyro raw to degrees per second
  //  rate_gyr_x = accelgyro.ngx;
  //  rate_gyr_y = accelgyro.ngy;
  //  rate_gyr_z = accelgyro.ngz;
  //  gyroXangle += rate_gyr_x * dT;
  //  gyroYangle += rate_gyr_y * dT;
  //  gyroZangle += rate_gyr_z * dT;

  // Roll angle
  angleX = accelGyroRelation * (-angleX + accelgyro.ngx * (float(dT) / 1000000)) + (1 - accelGyroRelation) *
           (atan2(accelgyro.nax, sqrt(pow(accelgyro.nay, 2) + pow(accelgyro.naz, 2))) * 180) / 3.14;
  angleX = - angleX;

  // Pitch angle
  angleY = accelGyroRelation * (-angleY + accelgyro.ngy * (float(dT) / 1000000)) + (1 - accelGyroRelation) *
           (atan2(accelgyro.nay, sqrt(pow(accelgyro.nax, 2) + pow(accelgyro.naz, 2))) * 180) / 3.14;
  angleY = - angleY;


  // Yaw Angle based on acelerometer
  //angleZ = accelGyroRelation * (angleZ + accelgyro.ngz * (float(dT) / 1000000)) + (1 - accelGyroRelation) *
  //         (atan2(sqrt(pow(accelgyro.nax, 2) + pow(accelgyro.nay, 2)), accelgyro.naz) * 180) / 3.14;

  //  mxCalibrated = my - ((myMax + myMin) / 2.0);
  //  myCalibrated = mx - ((mxMax + mxMin) / 2.0);
  //  mzCalibrated = mz - ((mzMax + mzMin) / 2.0);
  //
  //  angleZ = atan2( (mzCalibrated * sin(angleX) - myCalibrated * cos(angleX)),
  //                  (mxCalibrated * cos(angleY) + myCalibrated * sin(angleY) * sin(angleX) + mzCalibrated * sin(angleY) * cos(angleX))  ) * (180 / 3.14);

  // Yaw Angle Based on Compass
  compassCalculation();
  angleZ = mediaMovel(reads);
}


/* To calculate heading in degrees. 0 degree indicates North
   Calculate in radians
   xC = mx - ((mxMax + mxMin) / 2.0);
   yC = my - ((myMax + myMin) / 2.0);
   zC = mz - ((mzMax + mzMin) / 2.0);
*/
void compassCalculation() { //256us
  //calculation
  //Soft Iron and Hard Iron alibration
  float Sx = (float(myMax) - float(myMin))/(float(mxMax) - float(mxMin));
  float Sy = (float(mxMax) - float(mxMin))/(float(myMax) - float(myMin));

  if(Sx<1){
    Sx = 1;
  }
  if(Sy<1){
    Sy = 1;
  }
  
  float Ox = (float(mxMax - mxMin) / 2.0 - float(mxMax)) * Sx;
  float Oy = (float(myMax - myMin) / 2.0 - float(myMax)) * Sy;
  
//  Just Hard Iron
//  float Sx = 1;
//  float Sy = 1;
//  float Ox = float(mxMax + mxMin) / 2.0;
//  float Oy = float(myMax + myMin) / 2.0;
  compassAngle = atan2(Sy * (my + Oy), Sx * ( mx + Ox)) * (180 / PI);
  compassAngle = compassAngle - 180;


  Serial.print("\tSx: "); Serial.print(Sx);
  Serial.print("\tSy: "); Serial.print(Sy);
  Serial.print("\tOx: "); Serial.print(Ox);
  Serial.print("\tOy: "); Serial.println(Oy);
  
  //check if it north
  if (compassAngle >= 360) {
    compassAngle -= 360;
  }
  if (compassAngle < 0) {
    compassAngle += 360;
  }

  //pass it for an array to stabilizate the data
  for (int i = 0; i < mediaMovelArray - 1; i++) {
    reads[i] = reads[i + 1];
  }
  reads[mediaMovelArray - 1] = compassAngle;

}

/*
   Calculate the media of an array
*/
float mediaMovel(float *vetor) {
  float soma = 0;
  for (int i = 0; i < mediaMovelArray; i++) {
    soma += vetor[i];
  }
  return soma / mediaMovelArray;
}


void compassCalibration() {
  mxMax = 0;
  mxMin = 0;
  myMax = 0;
  myMin = 0;
  mzMax = 0;
  mzMin = 0;

  long unsigned int t_cal = millis();
  while ((millis() - t_cal) < (CALIBRATING_TIME * 1000)) {
    wdt_reset();
    if (bluetoothSerial.available() > 0) {
      bluetoothSerial.read(); //flush buffer11
      bluetoothSerial.print("c,c\n");
    }
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

#ifdef DEBUG
    Serial.print("Calibrating! Time remaining (s):");
    Serial.println(CALIBRATING_TIME - (millis() - t_cal) / 1000);
#endif
    delay(10);
  }

  EEPROM.write(0, mxMin >> 8);  EEPROM.write(1, mxMin % 256);
  EEPROM.write(2, myMin >> 8);  EEPROM.write(3, myMin % 256);
  EEPROM.write(4, mzMin >> 8);  EEPROM.write(5, mzMin % 256);
  EEPROM.write(6, mxMax >> 8);  EEPROM.write(7, mxMax % 256);
  EEPROM.write(8, myMax >> 8);  EEPROM.write(9, myMax % 256);
  EEPROM.write(10, mzMax >> 8);  EEPROM.write(11, mzMax % 256);

#ifdef DEBUG
  printCalibrationInfo();
#endif
}

void ressetCalibration() {
  mxMax = 0;
  mxMin = 0;
  myMax = 0;
  myMin = 0;
  mzMax = 0;
  mzMin = 0;
  EEPROM.write(0, mxMin >> 8);  EEPROM.write(1, mxMin % 256);
  EEPROM.write(2, myMin >> 8);  EEPROM.write(3, myMin % 256);
  EEPROM.write(4, mzMin >> 8);  EEPROM.write(5, mzMin % 256);
  EEPROM.write(6, mxMax >> 8);  EEPROM.write(7, mxMax % 256);
  EEPROM.write(8, myMax >> 8);  EEPROM.write(9, myMax % 256);
  EEPROM.write(10, mzMax >> 8);  EEPROM.write(11, mzMax % 256);
}
