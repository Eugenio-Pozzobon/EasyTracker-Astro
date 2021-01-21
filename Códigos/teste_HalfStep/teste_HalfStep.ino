#include <Stepper.h> //INCLUSÃO DE BIBLIOTECA
 
const int stepsPerRevolution = 60; //NÚMERO DE PASSOS POR VOLTA
 
Stepper myStepper(stepsPerRevolution, 6,10,7,11); //INICIALIZA O MOTOR UTILIZANDO OS PINOS DIGITAIS 8, 9, 10, 11
 
void setup(){
  myStepper.setSpeed(500); //VELOCIDADE DO MOTOR
}
void loop(){
  //LAÇO "for" QUE LIMITA O TEMPO EM QUE O MOTOR GIRA NO SENTIDO ANTI-HORÁRIO      
  myStepper.step(stepsPerRevolution); //GIRA O MOTOR NO SENTIDO ANTI-HORÁRIO
}
