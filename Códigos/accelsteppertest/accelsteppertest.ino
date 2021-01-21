// Include the AccelStepper Library
#include <AccelStepper.h>

// Define step constant
#define FULLSTEP 4

// Creates an instance
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
AccelStepper myStepper(FULLSTEP, 6, 10, 7, 11);

void setup() {
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  myStepper.setMaxSpeed(1000);
  myStepper.setAcceleration(1000);
  myStepper.setSpeed(1000);
 
}
int i = 6000;
void loop() {
  // Change direction once the motor reaches target position
  if (myStepper.distanceToGo() == 0) {
    myStepper.moveTo(i);
      i = -i;

  }
  // Move the motor one step
  myStepper.run();

}
