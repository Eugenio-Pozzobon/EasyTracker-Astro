#include <Stepper.h> //INCLUSÃO DE BIBLIOTECA
 
const int stepsPerRevolution = 2038; //NÚMERO DE PASSOS POR VOLTA
 
Stepper myStepper(stepsPerRevolution, 6,10,7,11); //INICIALIZA O MOTOR UTILIZANDO OS PINOS DIGITAIS 8, 9, 10, 11
 
void setup(){
 
}
void loop(){
  myStepper.setSpeed(100);
  for(int i = 0; i < stepsPerRevolution; i++){ //PARA "i" IGUAL A 0, ENQUANTO "i" MENOR QUE 50 INCREMENTA "i"
          myStepper.step(stepsPerRevolution); //GIRA O MOTOR NO SENTIDO ANTI-HORÁRIO
  }
   //LAÇO "for" QUE LIMITA O TEMPO EM QUE O MOTOR GIRA NO SENTIDO HORÁRIO            
  for(int i = 0; i < stepsPerRevolution; i++){//PARA "i" IGUAL A 0, ENQUANTO "i" MENOR QUE 50 INCREMENTA "i"
          myStepper.step(-stepsPerRevolution); //GIRA O MOTOR NO SENTIDO HORÁRIO
  }
}
