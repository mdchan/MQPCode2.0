#include <TimerOne.h>
#include <SparkFun_TB6612.h>

#define PWMA 6
#define AIN2 4
#define AIN1 2
#define STBY 8


const int offsetA = 1;
const int offsetB = 1;
int solenoidPin1 = 10;
int solenoidPin2 = 11;
int solenoidPin3 = 12;
int solenoidPin4 = 13;
int hallPin1 = 0;
int hallPin2 = 1;
int hallPin3 = 2;
int hallPin4 = 3;
int pressurePin = 4;
int potPin = 5;
int emgPin = 4;

Motor linMotor = Motor(AIN1, AIN2, PWMA, offsetA, STBY);

int hallRead1 = 0;
int hallRead2 = 0;
int hallRead3 = 0;
int hallRead4 = 0;
int pressureRead = 0;
int potRead = 0;
int emgRead = 0;

//rms stuff
int var = 0;
int rmsval = 0;
int rmstot = 0;
int rmsAvg = 0;
int rmsStore =0;
int bm1;
int bm2;
int storeRms;
boolean moving = false;
int rms[64];
int squareSum;
int counter = 0;
int i = 0;
char controlState;
int presThres = 50;

int oldHall[4] = {1000,1000,1000,1000};
int notFull[4] = {1,1,1,1};

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
  controlState = 4;


}

void loop() {
  //rms stuff
//  var = analogRead(emgPin)-400;
//  squareSum = squareSum +(var*var);
//  counter++;
//
//  if (counter>=64){
//    rmsval = sqrt(squareSum);
//    rmsStore=(rmsval/counter);
//    bm1 = bitRead(rmsStore,0);
//    bm2 = bitRead(rmsStore,1);
//    storeRms = bm1|bm2;
//    rms[i] = storeRms;
//    rmstot += storeRms;
//    squareSum = 0;
//    rmsval = 0;
//    counter = 0;
////    Serial.print(rms[i]);
////    Serial.print("\n");
//    i++;
//  }
//
//  if (i>=64){
//    if (rmstot<=50){
//    change this print to enter default state
////      Serial.print("code start");
////    Serial.print("\n");
//      moving = true;
//    }
//    else{
////  change this code to be passive state
////      Serial.print("waiting");
////      Serial.print("\n");
//      moving = false;
//    }
//    rmstot = 0;
//    i = 0;
//  }

  //this is the main control loop
  //while hall effects are activated, fills them up one by one until past point.

  switch (controlState){
    case 1:
    //open all sensors
    Serial.print("Entering Fill State");
    //while not calibrated, calibrate the bladder fill points
      sol1On();
      sol2On();
      sol3On();
      sol4On();
      readSensors();
      int fillState;
      fillState = 1;
      //fill each bladder individually, one at a time.
      if (fillState == 1){
        while (((oldHall[1] - hallRead1)<=50)){
          sol1Off();
          oldHall[1] = hallRead1;
          driveMotorFor();
        }
        sol1On();
        fillState = 2;
      }

      if (fillState == 2){
        while ((oldHall[2] - hallRead2)>=50){
          sol2Off();
          oldHall[2] = hallRead2;
          driveMotorFor();
        }
        sol2On();
        fillState = 3;
      }

      if (fillState ==3){
        while ((oldHall[3] - hallRead3)>=50){
          sol3Off;
          oldHall[3] = hallRead3;
          driveMotorFor();
        }
        sol3On();
        fillState = 4;
      }

      if (fillState == 4){
        while ((oldHall[4] - hallRead4)<=50){
          sol4Off();
          oldHall[4] = hallRead4;
          driveMotorFor();
        }
        sol4On();
      }

      motorCheck();
      break;


    case 2:
    Serial.print("Entering drain state");
    //drain state
    break;
    case 3:
    motorCheck();
    Serial.print("Entering passive state");
     while (hallRead1 <=800){
    sol1Off();
    Serial.print("emptying");
    Serial.print("\n");
    if (potRead >= 1024 || potRead <= 5){
    stopMotor();
    }
    else {
      driveMotorBack();
    }
     }
     
//     while (hallRead2 <=700){
    sol1Off();
    Serial.print("emptying");
    Serial.print("\n");
    if (potRead >= 1024 || potRead <= 5){
    stopMotor();
    }
    else {
      driveMotorBack();
    }
     }
     while (hallRead3 <=700){
    sol1Off();
    Serial.print("emptying");
    Serial.print("\n");
    if (potRead >= 1024 || potRead <= 5){
    stopMotor();
    }
    else {
      driveMotorBack();
    }
     }
     
     while (hallRead4 <=700){
    sol1Off();
    Serial.print("emptying");
    Serial.print("\n");
    if (potRead >= 1024 || potRead <= 5){
    stopMotor();
    }
    else {
      driveMotorBack();
    }
  }
    break;

  //default is active


  case 4:
  Serial.print(hallRead1);
  Serial.print("\n");
  //sustain case
  sol1On();
  sol2On();
  sol3On();
  sol4On();
  while (hallRead1 >=600){
    sol1Off();
    Serial.print("filling");
    Serial.print("\n");
    if (potRead >= 1024 || potRead <= 0){
    stopMotor();
    }
      driveMotorFor();
    
     }

//   while (hallRead2 >=600){
//    sol2Off();
//    Serial.print("filling");
//    Serial.print("\n");
//    if (potRead >= 1024 || potRead <= 0){
//    stopMotor();
//    }
//      driveMotorFor();
//     }
//
//  
//
//  while (hallRead3 >=600){
//    sol3Off();
//    Serial.print("filling");
//    Serial.print("\n");
//    if (potRead >= 1024 || potRead <= 0){
//    stopMotor();
//    }
//      driveMotorFor();
//  }
//
//  while (hallRead4 >=600){
//    sol4Off();
//    Serial.print("filling");
//    Serial.print("\n");
//    if (potRead >= 1024 || potRead <= 0){
//    stopMotor();
//    }
//    driveMotorFor();
//    
//  }
  readSensors();
  break;
  }
}



//Solenoid Helpers
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


//Motor Functions
void driveMotorBack(){
  linMotor.drive(-255,1);
  return;
}

void driveMotorFor(){
  linMotor.drive(255,1);
  return;
}

void stopMotor(){
  linMotor.brake();
  return;
}

void motorCheck(){
    //this is to prevent the linact from breaking
  if (potRead >= 1024 || potRead <= 5){
    stopMotor();
  }

}


//Read sensor interrupt
void readSensors(){
  hallRead1 = analogRead(hallPin1);
  hallRead2 = analogRead(hallPin2);
  hallRead3 = analogRead(hallPin3);
  hallRead4 = analogRead(hallPin4);
  pressureRead = analogRead(pressurePin);
  potRead = analogRead(potPin);
  
  return;
}
