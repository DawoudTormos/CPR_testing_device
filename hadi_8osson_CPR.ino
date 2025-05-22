#include "HX711.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
// HX711 Pins
#define DOUT1 3
#define CLK1 2
#define DOUT2 5
#define DOUT3 7
#define DOUT4 9

#define red 10
#define green 11

HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;

// Replace after calibration
float SCALE_FACTOR = 67528.18;

LiquidCrystal_I2C lcd(0x27, 16, 2);
int potpin = A0;
int greenLED = 6;
int redLED = 13;
int a;

int potValue = 0;
int maxPotValue = 0 ;
int potOffset = -8;
float f1=0;
float maxf1=0;
long unsigned int start = millis();
float displacement=0;
float correctDisplacement = 2;


void setupLEDandSerial(){
  Serial.begin(9600); // Start serial communication
  Serial.println("Potentiometer to Displacement (cm)");
  pinMode(red,OUTPUT);
  pinMode(green,OUTPUT);
}

void setupLCD() {
  lcd.init();
  lcd.backlight();
}
void setupDisplacement() {
  pinMode(potpin, INPUT);
}

void setupLoadCell() {
  Serial.println("Platform Scale (4 Load Cells)");
  // Initialize load cells
  scale1.begin(DOUT1, CLK1);
  // Set scale factors
  scale1.set_scale(SCALE_FACTOR);
  // Tare (zero out) all scales
  Serial.println("Taring all load cells...");
  scale1.tare();
  Serial.println("Tare complete.");
}

void displayValuesLCD(){
  lcd.clear(); // Clear previous value
  lcd.setCursor(0, 0);
  lcd.print("d: ");
  lcd.print(displacement);
  lcd.print(" w: ");
  lcd.print(maxf1);
  lcd.setCursor(0, 1);
  lcd.print(a);
}


void setup() {
  setupLEDandSerial();
  setupLoadCell();
  setupLCD();
  setupDisplacement();
}

void loop() {

  //measuring discplacement and press force. 
  potValue = analogRead(potpin) + potOffset ; // Read analog input (0–1023)
  f1 = scale1.get_units(1)*4.0;
  /*
  takes 85ms for the f1. 
  potValue measurement time is minimal(<1ms)
  */
  
  //checking for a new max with tthe new measurement
  if(potValue > maxPotValue){maxPotValue = potValue;}
  if(f1 > maxf1){maxf1 = f1;}

  if(millis() - start > 2000){


      // anaolgue to distance cm
      displacement = float(map(maxPotValue, 0, 290, 0, 600)) / 100;
      /*
      multiply 6 by 100 to be 600 then dividing by 100
      Doing this to get a float value with the map function that work only with integers and returns integrs.
      input is an int anyways.
      Other solution could be not using the map function at all.
      */
      


      // Display the result

      Serial.println("\n\n\n........\n");

      Serial.println("max analogue: "+ String(maxPotValue));
      Serial.print("max displacement: ");
      Serial.print(displacement, 2);
      Serial.println(" cm");
      Serial.print("Total Weight: ");
      Serial.print(maxf1, 2);
      Serial.println(" kg");

      Serial.println("");



      if(displacement > correctDisplacement && displacement < correctDisplacement + 0.5){

          Serial.println("keep going");
          digitalWrite(red,0);
          digitalWrite(green,1);
          displayValuesLCD();
          lcd.setCursor(0, 1);
          lcd.print("keep going");

      }else{

          Serial.println("Error! push harder");
          digitalWrite(red,1);
          digitalWrite(green,0);
          displayValuesLCD();
          lcd.setCursor(0, 1);
          lcd.print("Error! push harder");

      }
      Serial.println("");
      maxPotValue=0;
      maxf1= 0;
      start = millis();

  }
  

}
