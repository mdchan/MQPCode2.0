#include <Arduino.h>

#include <TimerOne.h>
#include <SparkFun_TB6612.h>

#define PWMA 6
#define AIN2 4
#define AIN1 2
#define STBY 8


const int offsetA = 1;
const int offsetB = 1;
int solenoidPin1 = 26;
int solenoidPin2 = 28;
int solenoidPin3 = 30;
int solenoidPin4 = 32;
int hallPin1 = 0;
int hallPin2 = 1;
int hallPin3 = 2;
int hallPin4 = 3;
int potPin = 4;
int emgPin = 5;
int pressurePin = 6;

Motor linMotor = Motor(AIN1, AIN2, PWMA, offsetA, STBY);

int hallRead1 = 0;
int hallRead2 = 0;
int hallRead3 = 0;
int hallRead4 = 0;
int pressureRead = 0;
int potRead = 0;
int emgRead = 0;

//rms stuff
int rmsval = 0;
int rmstot = 0;
int rmsAvg = 0;
int rmsStore =0;
int bm1;
int bm2;
int storeRms;
int rms[64];
int squareSum;
int counter = 0;
int i = 0;


char controlState;
int presThres = 50;

int oldHall[4] = {1000,1000,1000,1000};
int notFull[4] = {1,1,1,1};
int activeCounter = 0;
int passiveCounter = 0;


//hall rms stuff
int fill2count = 0;
int hallrmsval = 0;
int hallrmsStore = 0;
int hallSquare = 0;
int hallrms[64];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Timer1.initialize(5000);
  pinMode(solenoidPin1,OUTPUT);
  digitalWrite(solenoidPin1, LOW);
  pinMode(solenoidPin2,OUTPUT);
  digitalWrite(solenoidPin2, LOW);
  pinMode(solenoidPin3,OUTPUT);
  digitalWrite(solenoidPin3, LOW);
  pinMode(solenoidPin4,OUTPUT);
  digitalWrite(solenoidPin4, LOW);
  Timer1.attachInterrupt(readSensors,800);
  //linMotor.drive(255,200);
  controlState = 1;


}

void loop() {
  //this is the main control loop
  //while hall effects are activated, fills them up one by one until past point.

  switch (controlState){
    case 1:
    //open all sensors
      fillBladders();
      //Serial.print("exit fill");
      //if doesn't enter default
      controlState = 4;
    break;
    case 2:
    Serial.print("Entering passive state");
    Serial.print("\n");
    //passive state
    //if all set, then break and go back to rms reading
    controlState = 4;
    break;

  //default is active
    case 3:
    activeBladder();
  //if all set, then break and go back to rms reading
    controlState = 4;
    break;
//if need to, make this case 4
    //default:
    case 4:
    rmsRead();
    break;
  }
//might have to set a timer to check

  }


//sol off
void sol1Off(){
  digitalWrite(solenoidPin1, HIGH);
  return;
}

void sol2Off(){
  digitalWrite(solenoidPin2, HIGH);
  return;
}

void sol3Off(){
  digitalWrite(solenoidPin3, HIGH);
  return;
}

void sol4Off(){
  digitalWrite(solenoidPin4, HIGH);
  return;
}

//sol on
void sol1On(){
  digitalWrite(solenoidPin1, LOW);
  return;
}

void sol2On(){
  digitalWrite(solenoidPin2, LOW);
  return;
}

void sol3On(){
  digitalWrite(solenoidPin3, LOW);
  return;
}

void sol4On(){
  digitalWrite(solenoidPin4, LOW);
  return;
}

void driveMotorBack(){
  linMotor.drive(-255,1);
}

void driveMotorFor(){
  linMotor.drive(255,1);
}

void stopMotor(){
  linMotor.brake();
}

void motorCheck(){
    //this is to prevent the linact from breaking
  if (potRead >= 1024){
    stopMotor();
  }
  else{
    driveMotorBack();
  }
}

void fillBladders(){
//  Serial.print("Entering Fill State");
//  Serial.print("\n");
  //while not calibrated, calibrate the bladder fill points
    sol1On();
    sol2On();
    sol3On();
    sol4On();
    readSensors();
  
    
    int fillState;
    fillState = 1;
    int halldif;
    
    //fill each bladder individually, one at a time.
//    while (fillState == 1){
//          Serial.print(hallRead2);
//    Serial.print("\n");
//      readSensors();
//      halldif = oldHall[1] - hallRead1;
//      if (((halldif)>=50)){
//        sol1Off();
//        oldHall[1] = hallRead1;
//        driveMotorFor();        
//      }
////              Serial.print(oldHall[1]);
////        Serial.print("\n");
//
//    }

//      sol1On();
//      fillState = 2;
//    }

//    while (fillState == 2){
//      int element = 0;
//      driveMotorFor();      
//      hallSquare = hallSquare +(hallRead2);
//      fill2count++;
//      
//
//       if (fill2count>=100){
//        //hallrmsval = sqrt(hallSquare);
//        hallrmsStore=(hallrmsval/fill2count);
//        Serial.print(hallrmsStore);
//        Serial.print("\n");
//        hallrms[element] = hallrmsStore;
//        hallSquare = 0;
//        fill2count = 0;
//        hallrmsval = 0;
//        element++;
//       }
//
//      if (element>2){
//        if (hallrms[2]== hallrms[1]){
//          stopMotor();
//          sol2On();
//          Serial.print("exiting");
//          Serial.print("\n");
//          //fillState = 3;
//        }
//      }
//    }
      
//      if (halldif - oldhalldif <= 0){
//      Serial.print("here");
//      Serial.print("\n");
//      }
//      int fill2count = 0;
//      int pastOldHall = 0;
//      Serial.print(oldHall[2]);
//        Serial.print("\n");
//        Serial.print(pastOldHall);
//        Serial.print("\n");
//        readSensors();
//        int localHall2 = hallRead2;
//        halldif = oldHall[2] - localHall2;
//        sol2Off();
//        driveMotorFor();
//              Serial.print(halldif);
//        Serial.print("\n");
//      if ((halldif)>=3){
//        Serial.print("here");
//        Serial.print("\n");
//        pastOldHall = oldHall[2];
//        oldHall[2] = localHall2; 
//      }
//      if (halldif <= -10){
//        fill2count++;
//        if (fill2count >=10){
//        Serial.print("moving to 3");
//        Serial.print("\n");
//        }
        

//      }
      
      
//      sol2On();
 //     fillState = 3;
//    }
//while (fillState == 3){
//      Serial.print(oldHall[3]);
//        Serial.print("\n");
//        readSensors();
//        int localHall3 = hallRead3;
//        halldif = oldHall[3] - localHall3;
//        sol3Off();
//        driveMotorFor();
//      if ((halldif)>=5){
//        oldHall[3] = localHall3;
//      }
//      sol3On();
//      fillState = 4;
//    }
//    if (fillState == 4){
//      while ((oldHall[4] - hallRead4)>=50){
//        sol4Off();
//        oldHall[4] = hallRead4;
//        driveMotorFor();
//      }
//      sol4On();
//    }
    motorCheck();
    return;
}

void rmsRead(){
  // var = analogRead(emgPin)-400;
//  emgRead = analogRead(emgPin)-400;
  squareSum = squareSum +(emgRead*emgRead);
  counter++;
//  Serial.print("Entering Default EMG Reading state");
//  Serial.print("\n");
  if (counter>=64){
    rmsval = sqrt(squareSum);
    rmsStore=(rmsval/counter);
    bm1 = bitRead(rmsStore,0);
    bm2 = bitRead(rmsStore,1);
    storeRms = bm1|bm2;
    rms[i] = storeRms;
    rmstot += storeRms;
    squareSum = 0;
    rmsval = 0;
    counter = 0;
   //  Serial.print(rms[i]);
   //  Serial.print("\n");
    i++;
  }
  if (i>=64){

//    if (rmstot<=50){
// //if moving, enter sustain case
//      
//       Serial.print("code start");
//     Serial.print("\n");
//     controlState = 3;
//    }
//    else{
// //if not moving, enter passive state
//       Serial.print("waiting");
//       Serial.print("\n");
//      controlState = 2;
//    }

  if (rmstot<=50){
//      activeCounter ++;
//
//      if (activeCounter >= 1) 
      {
        //if moving, enter sustain case
        Serial.print("code start");
        Serial.print("\n");
        activeCounter = 0;
        passiveCounter = 0;
        controlState = 3;
      }
    }
    else{
      passiveCounter ++;
      //if not moving, enter passive state
      if (passiveCounter>=10){
        Serial.print("waiting");
        Serial.print("\n");
        passiveCounter = 0;
        controlState = 2;
      }
    }
    rmstot = 0;
    i = 0;
  }
}

void activeBladder(){
  Serial.print("entering active state");
  Serial.print("\n");
  //sustain case
  sol1On();
  sol2On();
  sol3On();
  sol4On();
  
motorCheck();
Serial.print(oldHall[1]);
    Serial.print("\n");
    while (hallRead1 >=600){
        if (potRead >= 1014){
    stopMotor();
        }
        else{
    sol1Off();
    Serial.print("driving");
    Serial.print("\n");
    driveMotorFor();
        }
    
  }

      while (hallRead2 >=600){
        if (potRead >= 1014){
    stopMotor();
        }
        else{
    sol2Off();
    Serial.print("driving");
    Serial.print("\n");
    driveMotorFor();
        }
    
  }

      while (hallRead3 >=600){
        if (potRead >= 1014){
    stopMotor();
        }
        else{
    sol3Off();
    Serial.print("driving");
    Serial.print("\n");
    driveMotorFor();
        }
    
  }

      while (hallRead4 >=600){
        if (potRead >= 1014){
    stopMotor();
        }
        else{
    sol4Off();
    Serial.print("driving");
    Serial.print("\n");
    driveMotorFor();
        }
    
  }

//  while (hallRead2 >=oldHall[2]){
//    sol2Off();
//    driveMotorFor();
//  }
//
//  while (hallRead3 >=oldHall[3]){
//    sol3Off();
//    driveMotorFor();
//  }
//
//  while (hallRead4 >=oldHall[4]){
//    sol4Off();
//    driveMotorFor();
//  }
Serial.print("exit active state");
  Serial.print("\n");
  readSensors();
  
  //return;
  //if all set, then break and go back to rms reading
}

void readSensors(){
  hallRead1 = analogRead(hallPin1);
  hallRead2 = analogRead(hallPin2);
  hallRead3 = analogRead(hallPin3);
  hallRead4 = analogRead(hallPin4);
  //pressureRead = analogRead(pressurePin);
  potRead = analogRead(potPin);
    emgRead = analogRead(emgPin)-400;


  return;
}
