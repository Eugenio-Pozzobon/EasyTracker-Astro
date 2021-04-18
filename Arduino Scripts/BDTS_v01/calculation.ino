void angleCalculation() {
  unsigned long cT = micros(); // contar tempo de loop
  unsigned long dT = cT - pT;
  pT = cT;

  rate_gyr_x = gx * G_GAIN;
  rate_gyr_y = gy * G_GAIN;
  rate_gyr_z = gz * G_GAIN;

  gyroXangle += rate_gyr_x * dT;
  gyroYangle += rate_gyr_y * dT;
  gyroZangle += rate_gyr_z * dT;

  AccXangle = (atan2(ax, sqrt(pow(ay, 2) + pow(az, 2))) * 180) / 3.14;
  AccYangle = (atan2(ay, sqrt(pow(ax, 2) + pow(az, 2))) * 180) / 3.14;
  AccZangle = (atan2(az, sqrt(pow(ax, 2) + pow(ay, 2))) * 180) / 3.14;

  CFangleX = AA * (CFangleX + rate_gyr_x * (double(dT) / 1000000)) + (1 - AA) * AccXangle;
  CFangleY = AA * (CFangleY + rate_gyr_y * (double(dT) / 1000000)) + (1 - AA) * AccYangle;
  CFangleZ = AA * (CFangleZ + rate_gyr_z * (double(dT) / 1000000)) + (1 - AA) * AccZangle;

  angleX = CFangleX;
  angleY = CFangleY;
  angleZ = CFangleZ;
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
    Serial.print("Calibrating! Time remaining (s):");
#ifdef BLUETOOTH
    mySerial.print("Calibrating!\n");
#endif
    Serial.println(CALIBRATING_TIME - (millis() - t_cal) / 1000);
    delay(10);
  }

  Serial.println("Calibration Infos:");
  Serial.print("mxMax: "); Serial.println(mxMax);
  Serial.print("mxMin: "); Serial.println(mxMin);
  Serial.print("myMax: "); Serial.println(myMax);
  Serial.print("myMin: "); Serial.println(myMin);
  Serial.print("mzMax: "); Serial.println(mzMax);
  Serial.print("mzMin: "); Serial.println(mzMin);
}

void compassCalculation() {
  // To calculate heading in degrees. 0 degree indicates North
  // corrige e calcula o angulo em radianos
  xC = mx - ((mxMax + mxMin) / 2.0);
  yC = my - ((myMax + myMin) / 2.0);
  zC = mz - ((mzMax + mzMin) / 2.0);

  float pitch = angleX * PI / 180;
  float roll = angleY * PI / 180;
  //float xh = xC * cos(pitch) + zC * sin(roll);
  //float yh = xC * sin(roll) * sin(pitch) + yC * cos(roll) - zC * sin(roll) * cos(pitch);
  //heading = atan(yh / xh);

  heading = atan2(yC, xC);
  heading_angle =  heading * 180 / PI + declination;
  if (heading_angle >= 360) {
    heading_angle -= 360;
  }
  if (heading_angle < 0) {
    heading_angle += 360;
  }
  for (int i = 0; i < mediaMovelArray- 1; i++) {
    reads[i] = reads[i + 1];
  }
  reads[mediaMovelArray - 1] = heading_angle;
}

float mediaMovel(float *vetor) {
  float soma = 0;
  for (int i = 0; i < mediaMovelArray; i++) {
    soma += vetor[i];
  }
  return soma / mediaMovelArray;
}
