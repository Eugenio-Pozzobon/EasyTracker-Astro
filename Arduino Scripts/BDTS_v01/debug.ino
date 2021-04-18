void debugSensor(){  
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
}