/*
bluetooth.ino - bluetooth control file of EasyTracker Astro System

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

/*
 * Initializate HC05 Serial Comunication
 */
void bluetoothSart() {
  bluetoothSerial.begin(9600);   //Serial Bluetooth
  pinMode(bluetoothStatePin, INPUT); //Enable pin HC05, indicate if it is connected with smartphone or not
}

/*
 * Manages Bluetooth Comunication
 */
void bluetoothCommunication() {

  bool bluetoothState = checkBluetoothCondition();
  
  if (bluetoothState = true) {
    
    if ( (bluetoothSerial.available() > 0) || ((millis() - timerBluetoothSendMessages) > 25) ) {//50Hz
      
      while (bluetoothSerial.available() > 0) {
        timerBluetoothRecieveMessages = millis(); // resset timer to countdown for get ACK response.
        timerBluetoothRestartMessages = millis();
        availableUserOption = bluetoothSerial.read(); //2ms for recieve a single char
        Serial.println(availableUserOption);
      }
      
      if ((millis() - timerBluetoothRecieveMessages) < 1000) { //it takes 16ms for send all angle buffer
        timerBluetoothSendMessages = millis(); // resset timer to countdown for send message.
        printDataBluetoothSerial(); //send messages to HC05 module
      }
      
    }

    if ((millis() - timerBluetoothRestartMessages) > 2000) { //if there is no communication with app (ACK) Try restart communication
      timerBluetoothRestartMessages = millis();
      printDataBluetoothSerial(); //send messages to HC05 module
    }
    
  }
  
}

void printDataBluetoothSerial() { //it takes 16ms for send this messages buffer
  bluetoothSerial.print(int(angleY * 10)); bluetoothSerial.print(",");
  bluetoothSerial.print(int(angleX * 10)); bluetoothSerial.print(",");
  bluetoothSerial.print(int(angleZ * 10)); bluetoothSerial.print(",");
  bluetoothSerial.print(int(angleX * 10) + int(angleY * 10) + int(angleZ * 10));
  bluetoothSerial.print("\n");
}
