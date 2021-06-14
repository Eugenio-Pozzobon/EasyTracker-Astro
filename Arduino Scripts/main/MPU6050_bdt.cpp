/*
  MPU6050.cpp - Class file for the MPU6050 Triple Axis Gyroscope & Accelerometer Arduino Library.

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

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include <math.h>

#include "MPU6050_bdt.h"

bool MPU6050::begin(mpu6050_dps_t scale, mpu6050_range_t range, mpu6050_adress adress, wire_speed speed) {
  // Address
  mpuAddress = adress;

  Wire.begin();

  Wire.setClock(speed);

  // Reset calibrate values
  dgx = 0;
  dgy = 0;
  dgz = 0;
  useCalibrate = false;

  // Reset threshold values
  tgx = 0;
  tgy = 0;
  tgz = 0;
  actualThreshold = 0;

  // Check MPU6050 Who Am I Register
  Wire.beginTransmission(mpuAddress);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  if (fastRegister8(MPU6050_REG_WHO_AM_I) != adress) {
    return false;
  }

  // Set Clock Source
  setClockSource(MPU6050_CLOCK_PLL_XGYRO);

  // Set Scale & Range
  setScale(scale);
  setRange(range);

  // Disable Sleep Mode
  setSleepEnabled(false);

  return true;
}

void MPU6050::setScale(mpu6050_dps_t scale) {
  int8_t value;

  switch (scale)
  {
    case MPU6050_SCALE_250DPS:
      dpsPerDigit = .007633f;
      break;
    case MPU6050_SCALE_500DPS:
      dpsPerDigit = .015267f;
      break;
    case MPU6050_SCALE_1000DPS:
      dpsPerDigit = .030487f;
      break;
    case MPU6050_SCALE_2000DPS:
      dpsPerDigit = .060975f;
      break;
    default:
      break;
  }

  value = readRegister8(MPU6050_REG_GYRO_CONFIG);
  value &= 0b11100111;
  value |= (scale << 3);
  writeRegister8(MPU6050_REG_GYRO_CONFIG, value);
}

mpu6050_dps_t MPU6050::getScale(void) {
  int8_t value;
  value = readRegister8(MPU6050_REG_GYRO_CONFIG);
  value &= 0b00011000;
  value >>= 3;
  return (mpu6050_dps_t)value;
}

void MPU6050::setRange(mpu6050_range_t range) {
  int8_t value;

  switch (range) {
    case MPU6050_RANGE_2G:
      rangePerDigit = .000061f;
      break;
    case MPU6050_RANGE_4G:
      rangePerDigit = .000122f;
      break;
    case MPU6050_RANGE_8G:
      rangePerDigit = .000244f;
      break;
    case MPU6050_RANGE_16G:
      rangePerDigit = .0004882f;
      break;
    default:
      break;
  }

  value = readRegister8(MPU6050_REG_ACCEL_CONFIG);
  value &= 0b11100111;
  value |= (range << 3);
  writeRegister8(MPU6050_REG_ACCEL_CONFIG, value);
}

mpu6050_range_t MPU6050::getRange(void) {
  int8_t value;
  value = readRegister8(MPU6050_REG_ACCEL_CONFIG);
  value &= 0b00011000;
  value >>= 3;
  return (mpu6050_range_t)value;
}

void MPU6050::setDHPFMode(mpu6050_dhpf_t dhpf) {
  int8_t value;
  value = readRegister8(MPU6050_REG_ACCEL_CONFIG);
  value &= 0b11111000;
  value |= dhpf;
  writeRegister8(MPU6050_REG_ACCEL_CONFIG, value);
}

void MPU6050::setDLPFMode(mpu6050_dlpf_t dlpf) {
  int8_t value;
  value = readRegister8(MPU6050_REG_CONFIG);
  value &= 0b11111000;
  value |= dlpf;
  writeRegister8(MPU6050_REG_CONFIG, value);
}

void MPU6050::setClockSource(mpu6050_clockSource_t source) {
  int8_t value;
  value = readRegister8(MPU6050_REG_PWR_MGMT_1);
  value &= 0b11111000;
  value |= source;
  writeRegister8(MPU6050_REG_PWR_MGMT_1, value);
}

mpu6050_clockSource_t MPU6050::getClockSource(void) {
  int8_t value;
  value = readRegister8(MPU6050_REG_PWR_MGMT_1);
  value &= 0b00000111;
  return (mpu6050_clockSource_t)value;
}

bool MPU6050::getSleepEnabled(void) {
  return readRegisterBit(MPU6050_REG_PWR_MGMT_1, 6);
}

void MPU6050::setSleepEnabled(bool state) {
  writeRegisterBit(MPU6050_REG_PWR_MGMT_1, 6, state);
}

bool MPU6050::getIntZeroMotionEnabled(void) {
  return readRegisterBit(MPU6050_REG_INT_ENABLE, 5);
}

void MPU6050::setIntZeroMotionEnabled(bool state) {
  writeRegisterBit(MPU6050_REG_INT_ENABLE, 5, state);
}

bool MPU6050::getIntMotionEnabled(void) {
  return readRegisterBit(MPU6050_REG_INT_ENABLE, 6);
}

void MPU6050::setIntMotionEnabled(bool state) {
  writeRegisterBit(MPU6050_REG_INT_ENABLE, 6, state);
}

bool MPU6050::getIntFreeFallEnabled(void) {
  return readRegisterBit(MPU6050_REG_INT_ENABLE, 7);
}

void MPU6050::setIntFreeFallEnabled(bool state) {
  writeRegisterBit(MPU6050_REG_INT_ENABLE, 7, state);
}

int8_t MPU6050::getMotionDetectionThreshold(void) {
  return readRegister8(MPU6050_REG_MOT_THRESHOLD);
}

void MPU6050::setMotionDetectionThreshold(int8_t threshold) {
  writeRegister8(MPU6050_REG_MOT_THRESHOLD, threshold);
}

int8_t MPU6050::getMotionDetectionDuration(void) {
  return readRegister8(MPU6050_REG_MOT_DURATION);
}

void MPU6050::setMotionDetectionDuration(int8_t duration) {
  writeRegister8(MPU6050_REG_MOT_DURATION, duration);
}

int8_t MPU6050::getZeroMotionDetectionThreshold(void) {
  return readRegister8(MPU6050_REG_ZMOT_THRESHOLD);
}

void MPU6050::setZeroMotionDetectionThreshold(int8_t threshold) {
  writeRegister8(MPU6050_REG_ZMOT_THRESHOLD, threshold);
}

int8_t MPU6050::getZeroMotionDetectionDuration(void) {
  return readRegister8(MPU6050_REG_ZMOT_DURATION);
}

void MPU6050::setZeroMotionDetectionDuration(int8_t duration) {
  writeRegister8(MPU6050_REG_ZMOT_DURATION, duration);
}

int8_t MPU6050::getFreeFallDetectionThreshold(void) {
  return readRegister8(MPU6050_REG_FF_THRESHOLD);
}

void MPU6050::setFreeFallDetectionThreshold(int8_t threshold)
{
  writeRegister8(MPU6050_REG_FF_THRESHOLD, threshold);
}

int8_t MPU6050::getFreeFallDetectionDuration(void) {
  return readRegister8(MPU6050_REG_FF_DURATION);
}

void MPU6050::setFreeFallDetectionDuration(int8_t duration) {
  writeRegister8(MPU6050_REG_FF_DURATION, duration);
}

bool MPU6050::getI2CMasterModeEnabled(void) {
  return readRegisterBit(MPU6050_REG_USER_CTRL, 5);
}

void MPU6050::setI2CMasterModeEnabled(bool state) {
  writeRegisterBit(MPU6050_REG_USER_CTRL, 5, state);
}

void MPU6050::setI2CBypassEnabled(bool state) {
  return writeRegisterBit(MPU6050_REG_INT_PIN_CFG, 1, state);
}

bool MPU6050::getI2CBypassEnabled(void) {
  return readRegisterBit(MPU6050_REG_INT_PIN_CFG, 1);
}

void MPU6050::setAccelPowerOnDelay(mpu6050_onDelay_t delay) {
  int8_t value;
  value = readRegister8(MPU6050_REG_MOT_DETECT_CTRL);
  value &= 0b11001111;
  value |= (delay << 4);
  writeRegister8(MPU6050_REG_MOT_DETECT_CTRL, value);
}

mpu6050_onDelay_t MPU6050::getAccelPowerOnDelay(void) {
  int8_t value;
  value = readRegister8(MPU6050_REG_MOT_DETECT_CTRL);
  value &= 0b00110000;
  return (mpu6050_onDelay_t)(value >> 4);
}

int8_t MPU6050::getIntStatus(void) {
  return readRegister8(MPU6050_REG_INT_STATUS);
}

Activites MPU6050::readActivites(void) {
  int8_t data = readRegister8(MPU6050_REG_INT_STATUS);

  a.isOverflow = ((data >> 4) & 1);
  a.isFreeFall = ((data >> 7) & 1);
  a.isInactivity = ((data >> 5) & 1);
  a.isActivity = ((data >> 6) & 1);
  a.isDataReady = ((data >> 0) & 1);

  data = readRegister8(MPU6050_REG_MOT_DETECT_STATUS);

  a.isNegActivityOnX = ((data >> 7) & 1);
  a.isPosActivityOnX = ((data >> 6) & 1);

  a.isNegActivityOnY = ((data >> 5) & 1);
  a.isPosActivityOnY = ((data >> 4) & 1);

  a.isNegActivityOnZ = ((data >> 3) & 1);
  a.isPosActivityOnZ = ((data >> 2) & 1);

}

void MPU6050::readRawAccel(void) {
  Wire.beginTransmission(mpuAddress);
#if ARDUINO >= 100
  Wire.write(MPU6050_REG_ACCEL_XOUT_H);
#else
  Wire.send(MPU6050_REG_ACCEL_XOUT_H);
#endif
  Wire.endTransmission(false);

  //Wire.beginTransmission(mpuAddress);
  Wire.requestFrom(mpuAddress, 6, true); // request a total of 14 registers

  //while (Wire.available() < 6);

#if ARDUINO >= 100
  rax = Wire.read() << 8 | Wire.read();
  ray = Wire.read() << 8 | Wire.read();
  raz = Wire.read() << 8 | Wire.read();
#else
  rax = Wire.recieve() << 8 | Wire.recieve();
  ray = Wire.recieve() << 8 | Wire.recieve();
  raz = Wire.recieve() << 8 | Wire.recieve();
#endif

}

void MPU6050::readNormalizeAccel(void) {
  readRawAccel();

  nax = rax * rangePerDigit * G;
  nay = ray * rangePerDigit * G;
  naz = raz * rangePerDigit * G;
}

void MPU6050::readScaledAccel(void) {
  readRawAccel();

  sax = rax * rangePerDigit;
  say = ray * rangePerDigit;
  saz = raz * rangePerDigit;
}


void MPU6050::readRawGyro(void) {
  Wire.beginTransmission(mpuAddress);
#if ARDUINO >= 100
  Wire.write(MPU6050_REG_GYRO_XOUT_H);
#else
  Wire.send(MPU6050_REG_GYRO_XOUT_H);
#endif
  Wire.endTransmission(false);

  //Wire.beginTransmission(mpuAddress);
  Wire.requestFrom(mpuAddress, 6, true);

  //while (Wire.available() < 6);

#if ARDUINO >= 100
  rgx = Wire.read() << 8 | Wire.read();
  rgy = Wire.read() << 8 | Wire.read();
  rgz = Wire.read() << 8 | Wire.read();
#else
  rgx = Wire.receive() << 8 | Wire.receive();
  rgy = Wire.receive() << 8 | Wire.receive();
  rgz = Wire.receive() << 8 | Wire.receive();
#endif

}

void MPU6050::readNormalizeGyro(void) {
  readRawGyro();

  if (useCalibrate) {
    ngx = (rgx - dgx) * dpsPerDigit;
    ngy = (rgy - dgy) * dpsPerDigit;
    ngz = (rgz - dgz) * dpsPerDigit;
  } else {
    ngx = rgx * dpsPerDigit;
    ngy = rgy * dpsPerDigit;
    ngz = rgz * dpsPerDigit;
  }

  if (actualThreshold) {
    if (abs(ngx) < tgx)
      ngx = 0;

    if (abs(ngy) < tgy)
      ngy = 0;

    if (abs(ngz) < tgz)
      ngz = 0;
  }
}

float MPU6050::readTemperature(void) {
  int16_t T;
  T = readRegister16(MPU6050_REG_TEMP_OUT_H);
  return (float)T / 340 + 36.53;
}

int16_t MPU6050::getGyroOffsetX(void) {
  return readRegister16(MPU6050_REG_GYRO_XOFFS_H);
}

int16_t MPU6050::getGyroOffsetY(void) {
  return readRegister16(MPU6050_REG_GYRO_YOFFS_H);
}

int16_t MPU6050::getGyroOffsetZ(void) {
  return readRegister16(MPU6050_REG_GYRO_ZOFFS_H);
}

void MPU6050::setGyroOffsetX(int16_t offset) {
  writeRegister16(MPU6050_REG_GYRO_XOFFS_H, offset);
}

void MPU6050::setGyroOffsetY(int16_t offset) {
  writeRegister16(MPU6050_REG_GYRO_YOFFS_H, offset);
}

void MPU6050::setGyroOffsetZ(int16_t offset) {
  writeRegister16(MPU6050_REG_GYRO_ZOFFS_H, offset);
}

int16_t MPU6050::getAccelOffsetX(void) {
  return readRegister16(MPU6050_REG_ACCEL_XOFFS_H);
}

int16_t MPU6050::getAccelOffsetY(void) {
  return readRegister16(MPU6050_REG_ACCEL_YOFFS_H);
}

int16_t MPU6050::getAccelOffsetZ(void) {
  return readRegister16(MPU6050_REG_ACCEL_ZOFFS_H);
}

void MPU6050::setAccelOffsetX(int16_t offset) {
  writeRegister16(MPU6050_REG_ACCEL_XOFFS_H, offset);
}

void MPU6050::setAccelOffsetY(int16_t offset) {
  writeRegister16(MPU6050_REG_ACCEL_YOFFS_H, offset);
}

void MPU6050::setAccelOffsetZ(int16_t offset) {
  writeRegister16(MPU6050_REG_ACCEL_ZOFFS_H, offset);
}

// Calibrate algorithm
void MPU6050::calibrateGyro(int16_t samples) {
  // Set calibrate
  useCalibrate = true;

  // Reset values
  float sumX = 0;
  float sumY = 0;
  float sumZ = 0;
  float sigmaX = 0;
  float sigmaY = 0;
  float sigmaZ = 0;

  // Read n-samples
  for (int16_t i = 0; i < samples; ++i)
  {
    readRawGyro();
    sumX += rgx;
    sumY += rgy;
    sumZ += rgz;

    sigmaX += rgx * rgx;
    sigmaY += rgy * rgy;
    sigmaZ += rgz * rgz;

    delay(5);
  }

  // Calculate delta vectors
  dgx = sumX / samples;
  dgy = sumY / samples;
  dgz = sumZ / samples;

  // Calculate threshold vectors
  thx = sqrt((sigmaX / 50) - (dgx * dgx));
  thy = sqrt((sigmaY / 50) - (dgy * dgy));
  thz = sqrt((sigmaZ / 50) - (dgz * dgz));

  // If already set threshold, recalculate threshold vectors
  if (actualThreshold > 0)
  {
    setThreshold(actualThreshold);
  }
}

// Get current threshold value
int16_t MPU6050::getThreshold(void) {
  return actualThreshold;
}

// Set treshold value
void MPU6050::setThreshold(int16_t multiple) {
  if (multiple > 0)
  {
    // If not calibrated, need calibrate
    if (!useCalibrate)
    {
      calibrateGyro();
    }

    // Calculate threshold vectors
    tgx = thx * multiple;
    tgy = thy * multiple;
    tgz = thz * multiple;
  } else
  {
    // No threshold
    tgx = 0;
    tgy = 0;
    tgz = 0;
  }

  // Remember old threshold value
  actualThreshold = multiple;
}

// Fast read 8-bit from register
int8_t MPU6050::fastRegister8(int8_t reg) {
  int8_t value;

  Wire.beginTransmission(mpuAddress);
#if ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif
  Wire.endTransmission();

  Wire.beginTransmission(mpuAddress);
  Wire.requestFrom(mpuAddress, 1);
#if ARDUINO >= 100
  value = Wire.read();
#else
  value = Wire.receive();
#endif;
  Wire.endTransmission();

  return value;
}

// Read 8-bit from register
int8_t MPU6050::readRegister8(int8_t reg) {
  int8_t value;

  Wire.beginTransmission(mpuAddress);
#if ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif
  Wire.endTransmission();

  Wire.beginTransmission(mpuAddress);
  Wire.requestFrom(mpuAddress, 1);
  while (!Wire.available()) {};
#if ARDUINO >= 100
  value = Wire.read();
#else
  value = Wire.receive();
#endif;
  Wire.endTransmission();

  return value;
}

// Write 8-bit to register
void MPU6050::writeRegister8(int8_t reg, int8_t value) {
  Wire.beginTransmission(mpuAddress);

#if ARDUINO >= 100
  Wire.write(reg);
  Wire.write(value);
#else
  Wire.send(reg);
  Wire.send(value);
#endif
  Wire.endTransmission();
}

int16_t MPU6050::readRegister16(int8_t reg) {
  int16_t value;
  Wire.beginTransmission(mpuAddress);
#if ARDUINO >= 100
  Wire.write(reg);
#else
  Wire.send(reg);
#endif
  Wire.endTransmission();

  Wire.beginTransmission(mpuAddress);
  Wire.requestFrom(mpuAddress, 2);
  while (!Wire.available()) {};
#if ARDUINO >= 100
  int8_t vha = Wire.read();
  int8_t vla = Wire.read();
#else
  int8_t vha = Wire.receive();
  int8_t vla = Wire.receive();
#endif;
  Wire.endTransmission();

  value = vha << 8 | vla;

  return value;
}

void MPU6050::writeRegister16(int8_t reg, int16_t value) {
  Wire.beginTransmission(mpuAddress);

#if ARDUINO >= 100
  Wire.write(reg);
  Wire.write((int8_t)(value >> 8));
  Wire.write((int8_t)value);
#else
  Wire.send(reg);
  Wire.send((int8_t)(value >> 8));
  Wire.send((int8_t)value);
#endif
  Wire.endTransmission();
}

// Read register bit
bool MPU6050::readRegisterBit(int8_t reg, int8_t pos) {
  int8_t value;
  value = readRegister8(reg);
  return ((value >> pos) & 1);
}

// Write register bit
void MPU6050::writeRegisterBit(int8_t reg, int8_t pos, bool state) {
  int8_t value;
  value = readRegister8(reg);

  if (state) {
    value |= (1 << pos);
  } else {
    value &= ~(1 << pos);
  }

  writeRegister8(reg, value);
}
