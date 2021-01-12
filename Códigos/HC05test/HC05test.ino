//Bluetooth
#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 3); // RX/TX 

void setup() {
  // put your setup code here, to run once:
  mySerial.begin(9600);   //Serial Bluetooth
  Serial.begin(115200);
  //pinMode(2, OUTPUT);
  pinMode(2, INPUT);
  //digitalWrite(2, LOW);
  pinMode(5, INPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  mySerial.println("Hello Word");
  Serial.print(digitalRead(5));
  Serial.println(digitalRead(2));
  if(Serial.available()){
    digitalWrite(2, HIGH);
    delay(10);
  }else{
    //digitalWrite(2, LOW);
  }
  //digitalWrite(2, LOW);
}
