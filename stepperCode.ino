#include <AccelStepper.h>
#include <ezButton.h>
#include <LiquidCrystal.h>

//----- DEFINING PERMANENT VALUES -----
#define killSwitch 1
#define microPin 9
const unsigned short StepsPerRev = 400;
const float circ = 0.2199; //In meters
const float dist = 1.5; //In meters
const float calibrationFactor = 1.031; //Used to compensate for some mechanical elasticity
const float extraAcceleration = 500; //This variable is sett to compensate for lower torque at high rpm.
const float backwardSpeed = 150;
const float maxSpeedVal = 2.5; //2.5 m/s
const float incr = 0.1; //increasing valuie

//----- SETTING PINS -----
AccelStepper stepper(1,7,6); //Defines stepper pins
ezButton interButGo(8); //Sends stone

LiquidCrystal lcd(12, 11, 5, 4, 3, 13); //LCD-display
ezButton interButUp(10); //Fart++
ezButton interButDown(2);//Fart--

//----- DEFINING GLOBAL VARIABLES -----
bool forward = false;
bool forwardSet = false;
bool backward = true;
float fart; //variable for speed

//----- DEFINING FUNCTIONS -----
float calcStepDist(float l, unsigned short SPR, float circumference){ //l in meters
  return (l*SPR/circumference);
}

float calcStepSpeed(float v, float circumference, unsigned short SPR){
  float rps = v/circumference;
  return (SPR*rps);
}

float calcStepAccelration(float stepSpeed, float stepDist){
  return (stepSpeed*stepSpeed/(2*stepDist));
}

const float distInSteps = calcStepDist(dist, StepsPerRev, circ*calibrationFactor); //defining number of steps to reach a distance
float stepVelocity = calcStepSpeed(1, circ*calibrationFactor, StepsPerRev);
float stepAccelration = calcStepAccelration(stepVelocity, distInSteps);

void setThrowAcc(float aStep, float vStep, float lStep){ //setting acceleration for the stone
 interButGo.loop();
  if(interButGo.isPressed()){
      stepper.setAcceleration(aStep+extraAcceleration);
      stepper.setMaxSpeed(vStep);
      stepper.moveTo(lStep);
      forwardSet = true;
  }
}

void returnStone(float backSpeed){
  stepper.setSpeed(-backSpeed);
  stepper.runSpeed();
}

//Fart++ , fart-- funksjon
float adjustSpeed(){
  interButUp.loop();
  interButDown.loop();
  
  if(interButUp.isPressed()){ //up
     fart+=incr;
     if(fart<=maxSpeedVal){
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print(fart);
        lcd.print("m/s");
        return fart;
      }
     fart-=incr;
     lcd.clear();
     lcd.setCursor(5,0);
     lcd.print(fart);
     lcd.print("m/s");
     return fart;  
  }
  if(interButDown.isPressed()){ //down
      fart-=incr;
      if(fart>0){
        lcd.clear();
        lcd.setCursor(5,0);
        lcd.print(fart);
        lcd.print("m/s");
        return fart;
      }
      fart+=incr;
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print(fart);
      lcd.print("m/s");
      return fart; 
  }
  return fart;
}
//----- SETUP FUNCTION -----

void setup() {
  //button setup
  pinMode(microPin, INPUT);
  pinMode(killSwitch, INPUT);
  interButGo.setDebounceTime(50);
  
  //Motor setup
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(1000);
  
  ///LCD Screen setup
  lcd.begin(16,2);
  lcd.setCursor(5, 0);
  lcd.print("Fart: ");
  lcd.setCursor(5,1);
  lcd.print(fart);
  lcd.print("m/s");
  
}


//----- MAIN LOOP -----

void loop() {
  if(digitalRead(killSwitch)== HIGH){
    if(forward){
      if(!forwardSet){
          stepVelocity = calcStepSpeed(adjustSpeed(), circ*calibrationFactor, StepsPerRev);
          stepAccelration = calcStepAccelration(stepVelocity, distInSteps);
          setThrowAcc(stepAccelration, stepVelocity, distInSteps);
      }else{
        stepper.run();
      }
      if(stepper.distanceToGo()==1){
        backward = true;
        stepper.setCurrentPosition(distInSteps);
        forward = false;
        delay(10000);
      }
    }
    if (backward){
      if(digitalRead(microPin) == HIGH){
        returnStone(backwardSpeed);
      }else{
        stepper.setCurrentPosition(0);
        backward = false;
        forward = true;
        forwardSet = false;
      }
    }
  } else {
    backward = false;
    forward = false;
    forwardSet = false;
    lcd.clear();
    lcd.print("RESET!");
  }
}
