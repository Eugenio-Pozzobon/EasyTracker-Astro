void angleCalculation() {
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
}

void compassCalibration() {
  long unsigned int t_cal = millis();
  while ((millis() - t_cal) < (CALIBRATING_TIME*1000)) {
    mag.getHeading(&mx, &my, &mz);
    if (mxMax < mx) {
      mxMax = mx;
    }
    if (myMax < my) {
      myMax = my;
    }
    if (mxMin > mx) {
      mxMin = mx;
    }
    if (myMin > my) {
      myMin = my;
    }
    Serial.print("Calibrating! Time remaining (s):");
    Serial.println(CALIBRATING_TIME-(millis() - t_cal)/1000);
    delay(10);
  }

  Serial.println("Calibration Infos:");
  Serial.print("mxMax: "); Serial.println(mxMax);
  Serial.print("mxMin: "); Serial.println(mxMin);
  Serial.print("myMax: "); Serial.println(myMax);
  Serial.print("myMin: "); Serial.println(myMin);
}

void compassCalculation() {
  // To calculate heading in degrees. 0 degree indicates North
  // corrige e calcula o angulo em radianos
  xC = mx - ((mxMax + mxMin) / 2.0);
  yC = my - ((myMax + myMin) / 2.0);
  heading = atan2(yC, xC);
  heading_angle =  heading * 180 / PI + declination;
  
  if (heading_angle < 0) {
    heading_angle += 360;
  } else if (heading_angle >= 360) {
    heading_angle -= 360;
  }
}
