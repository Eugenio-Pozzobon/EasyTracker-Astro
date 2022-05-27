/*
debug.ino - debug process file of EasyTracker Astro

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
