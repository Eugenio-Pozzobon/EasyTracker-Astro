void angleCalculation() { //676us usando int_16t e 1100us usando float
  unsigned long cT = micros(); // contar tempo de loop
  int dT = cT - pT; //Variable for measure angle with gps and make integration of values
  pT = cT;

  //  rate_gyr_x = accelgyro.ngx * G_GAIN;
  //  rate_gyr_y = accelgyro.ngy * G_GAIN;
  //  rate_gyr_z = accelgyro.ngz * G_GAIN;
  //  gyroXangle += rate_gyr_x * dT;
  //  gyroYangle += rate_gyr_y * dT;
  //  gyroZangle += rate_gyr_z * dT;

  angleX = AA * (angleX + accelgyro.ngx * G_GAIN * (float(dT) / 1000000)) + (1 - AA) *
           (atan2(accelgyro.nax, sqrt(pow(accelgyro.nay, 2) + pow(accelgyro.naz, 2))) * 180) / 3.14;
  angleY = AA * (angleY + accelgyro.ngy * G_GAIN * (float(dT) / 1000000)) + (1 - AA) *
           (atan2(accelgyro.nay, sqrt(pow(accelgyro.nax, 2) + pow(accelgyro.naz, 2))) * 180) / 3.14;
  angleZ = AA * (angleZ + accelgyro.ngz * G_GAIN * (float(dT) / 1000000)) + (1 - AA) *
           (atan2(accelgyro.naz, sqrt(pow(accelgyro.nax, 2) + pow(accelgyro.nay, 2))) * 180) / 3.14;
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
#ifdef WHATCHDOG
    wdt_reset();
#endif
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

/* To calculate heading in degrees. 0 degree indicates North
   Calculate in radians
   xC = mx - ((mxMax + mxMin) / 2.0);
   yC = my - ((myMax + myMin) / 2.0);
   zC = mz - ((mzMax + mzMin) / 2.0);
*/
void compassCalculation() { //256us
  //calculation
  heading_angle =  atan2(my - ((myMax + myMin) / 2.0), mx - ((mxMax + mxMin) / 2.0)) * (180 / PI);

  //check if it north
  if (heading_angle >= 360) {
    heading_angle -= 360;
  }
  if (heading_angle < 0) {
    heading_angle += 360;
  }

  //pass it for an array to stabilizate the data
  for (int i = 0; i < mediaMovelArray - 1; i++) {
    reads[i] = reads[i + 1];
  }
  reads[mediaMovelArray - 1] = heading_angle;

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
