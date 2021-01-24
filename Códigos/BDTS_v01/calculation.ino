/*void angleCalculation() {
  //X-AXIS
  denomX_A = pow(ay, 2);
  denomX_B = pow(az, 2);
  denomX_C = denomX_A + denomX_B;
  denomX_T = pow(denomX_C, .5);                   //pow returns base raised to the power exponent
  angleX = atan(ax / denomX_T) * 180 / PI;       //should calculate the angle on the X axis in
  //degrees based on raw data

  //Y-AXIS
  denomY_A = pow(ax, 2);
  denomY_B = pow(az, 2);
  denomY_C = denomY_A + denomY_B;
  denomY_T = pow(denomY_C, .5);                   //pow returns base raised to the power exponent
  angleY = atan(ay / denomY_T) * 180 / PI;       //should calculate the angle on the Y axis in degrees based
  // on raw data

  //Z-AXIS
  denomZ_A = pow(ax, 2);
  denomZ_B = pow(ay, 2);
  denomZ_C = denomZ_A + denomZ_B;
  denomZ_T = pow(denomZ_C, .5);
  angleZ = atan(az / denomY_T) * 180 / PI;
  }*/


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

  CFangleX = AA * (CFangleX + rate_gyr_x * (dT / 1000000)) + (1 - AA) * AccXangle;
  CFangleY = AA * (CFangleY + rate_gyr_y * (dT / 1000000)) + (1 - AA) * AccYangle;
  CFangleZ = AA * (CFangleZ + rate_gyr_z * (dT / 1000000)) + (1 - AA) * AccZangle;

  angleX = CFangleX;
  angleY = CFangleY;
  angleZ = CFangleZ;
}



void compassCalibration() {

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
    delay(100);
  }

  Serial.println("Calibration Infos:");
  Serial.print("mxMax: "); Serial.println(mxMax);
  Serial.print("mxMin: "); Serial.println(mxMin);
  Serial.print("myMax: "); Serial.println(myMax);
  Serial.print("myMin: "); Serial.println(myMin);
  Serial.print("mzMax: "); Serial.println(mzMax);
  Serial.print("mzMin: "); Serial.println(mzMin);

  //mxMax: 172
  //mxMin: -173
  //myMax: 381
  //myMin: 0
  /*
    mxMax = 172;
    mxMin = -173;
    myMax = 381;
    myMin = 0;

    00:02:59.247 -> mxMax: 475
    00:02:59.247 -> mxMin: 0
    00:02:59.247 -> myMax: 5
    00:02:59.247 -> myMin: -361

    FIO PRÓXIMO -> 5° DE ERRRO!!!!!!

  */
}

void compassCalculation() {
  // To calculate heading in degrees. 0 degree indicates North
  // corrige e calcula o angulo em radianos
  xC = mx - ((mxMax + mxMin) / 2.0);
  yC = my - ((myMax + myMin) / 2.0);
  zC = mz - ((mzMax + mzMin) / 2.0);

  float pitch = angleX * PI / 180;
  float roll = angleY * PI / 180;
  float xh = xC * cos(pitch) + zC * sin(roll);
  float yh = xC * sin(roll) * sin(pitch) + yC * cos(roll) - zC * sin(roll) * cos(pitch);
  heading = atan(yh / xh);

  //heading = atan2(yC, xC);
  heading_angle =  heading * 180 / PI + declination;
  if (heading_angle >= 360) {
    heading_angle -= 360;
  }
  if (heading_angle < 0) {
    heading_angle += 360;
  }

  for (int i = 0; i < 9; i++) {
    reads[i] = reads[i + 1];
  }
  reads[9] = heading_angle;
}

float mediaMovel(float *vetor) {
  float soma = 0;
  for (int i = 0; i < 10; i++) {
    soma += vetor[i];
  }
  return soma / 10;
}
