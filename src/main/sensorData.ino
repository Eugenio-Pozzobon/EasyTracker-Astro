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
  //(atan2(accelgyro.nay, accelgyro.naz) * 180) / 3.14;

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
  compassAngle = atan2(my - ((myMax + myMin) / 2.0), mx - ((mxMax + mxMin) / 2.0)) * (180 / PI);
  compassAngle = compassAngle + 180;

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
