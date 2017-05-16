#include <Arduino.h>

#include <TimerOne.h>
#include <SparkFun_TB6612.h>

#define PWMA 2
#define AIN2 4
#define AIN1 6
#define STBY 8


const int offsetA = 1;
const int offsetB = 1;
int solenoidPin1 = 27;
int solenoidPin2 = 29;
int solenoidPin3 = 31;
int solenoidPin4 = 33;
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


int maxPressure = 150;


//hall rms stuff
int fill2count = 0;
int hallrmsval = 0;
int hallrmsStore = 0;
int hallSquare = 0;
int hallrms[64];

int maxpot = 900;
int minpot = 200;

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
  Timer1.attachInterrupt(readSensors,1000);
  //linMotor.drive(255,200);
  controlState = 1;


}

void loop() {
  //this is the main control loop
  //while hall effects are activated, fills them up one by one until past point.

//  Serial.print(pressureRead);
//  Serial.print("\t");
//   Serial.print(hallRead1);
//  Serial.print("\t");
//   Serial.print(hallRead2);
//  Serial.print("\t");
//   Serial.print(hallRead3);
//  Serial.print("\t");
//  Serial.print(hallRead4);
//Serial.print("\t");
//  Serial.print(potRead);
//  Serial.print("\n");
//
  //emptyBladders();

        
  switch (controlState){
    case 1:
    //open all sensors
      fillBladders();
      //Serial.print("exit fill");
      //if doesn't enter default
      //controlState = 4;
    break;
    case 2:
    //passive state
    passiveState();
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

int driveMotorBack(int driveBacktime){
  linMotor.drive(-255,driveBacktime);
}

int driveMotorFor(int driveFortime){
  linMotor.drive(255,driveFortime);
}

void stopMotor(){
  linMotor.brake();
}

void motorCheck(){
    //this is to prevent the linact from breaking
  if (potRead >= maxpot-10 || potRead < minpot+10){
    stopMotor();
  }
  else{
    driveMotorBack(1);
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
    
    Serial.print(pressureRead);
  Serial.print("\n");
    
    int fillState;
    fillState = 1;
    int halldif;
    
    //fill each bladder individually, one at a time.
      
    while (fillState == 1){
      sol1Off();
      if (pressureRead<maxPressure){
        if (potRead >= maxpot){
          stopMotor();
        }
        else{
        driveMotorFor(250);
//        Serial.print("Driving motor");
//        Serial.print("\n");
        fillState = 2;
      }
      }
      else{
        //if pressure spike, set oldHall to the current hallread value
        oldHall[1] = hallRead1;
        sol1On();
//        Serial.print("Pressure Spiked!");
//        Serial.print("\n");
        fillState = 2;
        break;
      }
    }

    while (fillState == 2){
      sol2Off();
      if (pressureRead<maxPressure){
        if (potRead >= maxpot){
          stopMotor();
        }
        else{
        driveMotorFor(250);
//        Serial.print("Driving motor");
//        Serial.print("\n");
        fillState = 3;
      }
      }
      else{
        //if pressure spike, set oldHall to the current hallread value
        oldHall[2] = hallRead2;
        sol2On();
//        Serial.print("Pressure Spiked!");
//        Serial.print("\n");
        fillState = 3;
        break;
      }
    }
    
while (fillState == 3){
      sol3Off();
      if (pressureRead<maxPressure){
        if (potRead >= maxpot){
          stopMotor();
        }
        else{
        driveMotorFor(250);
//        Serial.print("Driving motor");
//        Serial.print("\n");
        fillState = 4;
      }
      }
      else{
        //if pressure spike, set oldHall to the current hallread value
        oldHall[3] = hallRead3;
        sol3On();
//        Serial.print("Pressure Spiked!");
//        Serial.print("\n");
        fillState = 4;
        break;
      }
}
    
while (fillState == 4){
      sol4Off();
      if (pressureRead<maxPressure){
        if (potRead >= maxpot){
          stopMotor();
        }
        else{
        driveMotorFor(250);
//        Serial.print("Driving motor");
//        Serial.print("\n");
        fillState =1;
      }
      }
      else{
        //if pressure spike, set oldHall to the current hallread value
        oldHall[4] = hallRead4;
        sol4On();
//        Serial.print("Pressure Spiked!");
//        Serial.print("\n");
        break;
      }
}
//    motorCheck();
      return;
}

void passiveState(){
  Serial.print("entering passive state");
  Serial.print("\n");
  //sustain case

  sol1On();
  sol2On();
  sol3On();
  sol4On();
  
//motorCheck();
    while (hallRead1 <= oldHall[1]+5){
          sol1Off();
        if (potRead <= minpot){
          stopMotor();
        }
        else{

//    Serial.print(hallRead1);
//    Serial.print("\t");
//    Serial.print(oldHall[1]);
//    Serial.print("\n");
    driveMotorBack(200);
    
    
        }
        sol1On();
        break;
        
    
  }

    while (hallRead2 <= oldHall[2]+5){
          sol2Off();
        if (potRead <= minpot){
    stopMotor();
        }
        else{

    Serial.print("oldHall2");
    Serial.print("\n");
    driveMotorBack(200);
    
    
        }
        sol2On();
        break;
        
    
  }

    while (hallRead3 <= oldHall[3]+5){
          sol3Off();
        if (potRead <= minpot){
    stopMotor();
        }
        else{

    Serial.print("oldHall3");
    Serial.print("\n");
    driveMotorBack(200);
    
    
        }
        sol3On();
        break;
    
  }

      while (hallRead4  <= oldHall[4]+5){
        sol4Off();
        if (potRead <= minpot){
    stopMotor();
        }
        else{
    Serial.print("oldHall4");
    Serial.print("\n");
    driveMotorBack(200);
    
        }
    sol4On();
    break;
    
  }
  Serial.print("exiting passive state");
  Serial.print("\n");


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
  
//motorCheck();
//    while (abs(oldHall[1] - hallRead1)  >= 10){
while (oldHall[1] < hallRead1){
          sol1Off();
        if (potRead >= maxpot){
          stopMotor();
        }
        else{

    Serial.print("oldhall1");
    Serial.print("\t");
//    Serial.print("1diff is ");
//    Serial.print(oldHall[1] - hallRead1);
//    Serial.print("\n");
        driveMotorFor(300);
        
    
        }
        sol1On();
        break;
        
        
    
  }

    while (oldHall[2] < hallRead2){
          sol2Off();
        if (potRead >= maxpot){
    stopMotor();
        }
        else{

    Serial.print("oldhall2");
    Serial.print("\n");
//    Serial.print("2 diff is ");
//    Serial.print(oldHall[2] - hallRead2);
//    Serial.print("\n");
    driveMotorFor(200);
    
        
    
        }
        sol2On();
        break;
        
    
  }

    while (oldHall[3] < hallRead3){
          sol3Off();
        if (potRead >= maxpot){
    stopMotor();
        }
        else{

    Serial.print("oldhall3 ");
    driveMotorFor(300);
    
    
        }
        sol3On();
        break;
        
    
  }

      while (oldHall[4] < hallRead4){
        sol4Off();
        if (potRead >= maxpot){
    stopMotor();
        }
        else{
    Serial.print("oldhall4 ");
    Serial.print("\n");
    driveMotorFor(300);
    
        }
        sol4On();
    break;
    
    
  }
  Serial.print("exiting active state");
  Serial.print("\n");

  sol1On();
  sol2On();
  sol3On();
  sol4On();
  
}


void emptyBladders(){
  sol1Off();
  sol2Off();
  sol3Off();
  sol4Off();
  driveMotorBack(1000);
}





void readSensors(){
  hallRead1 = analogRead(hallPin1);
  hallRead2 = analogRead(hallPin2);
  hallRead3 = analogRead(hallPin3);
  hallRead4 = analogRead(hallPin4);
  pressureRead = analogRead(pressurePin);
  potRead = analogRead(potPin);
  emgRead = analogRead(emgPin)-400;


  return;
}
