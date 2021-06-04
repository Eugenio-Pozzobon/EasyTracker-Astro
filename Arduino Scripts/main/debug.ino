void debugSensor() {
#ifdef DEBUG
  Serial.print("\tRoll: ");
  Serial.print(angleY);
  Serial.print("\tPitch: ");
  Serial.print(angleX);
  Serial.print("\tYaw: ");
  Serial.println(angleZ);
#endif
}
void printCalibrationInfo() {
  #ifdef DEBUG
  Serial.println("Calibration Infos:");
  Serial.print("mxMax: "); Serial.println(mxMax);
  Serial.print("mxMin: "); Serial.println(mxMin);
  Serial.print("myMax: "); Serial.println(myMax);
  Serial.print("myMin: "); Serial.println(myMin);
  Serial.print("mzMax: "); Serial.println(mzMax);
  Serial.print("mzMin: "); Serial.println(mzMin);
#endif
}
