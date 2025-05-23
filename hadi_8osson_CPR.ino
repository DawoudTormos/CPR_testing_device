#include "HX711.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Pins
#define DOUT1 3
#define CLK1 2
#define red 10
#define green 11

// Constants
const float SCALE_FACTOR = 67528.18;  
const float CORRECT_DISPLACEMENT = 2.0;  // Target compression depth (cm)
const int WINDOW_SIZE = 5;           // Moving average window size
const float MIN_COMPRESSION_DEPTH = 0.6;  
const int DISPLAY_INTERVAL = 5000;   // Update display every 2 seconds
const int minBpm = 50, maxBpm = 80; // BPM limits

// Objects
HX711 scale1;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
int potValue = 0;
int maxPotValue = 0;
int potOffset = -8;
float f1 = 0;
float maxf1 = 0;
float displacement = 0;
float maxdisplacement = 0;
float displacements[WINDOW_SIZE] = {0};
float smoothed = 0;
int bpm = 100;
int count = 0;
unsigned long lastDisplayUpdate = 0;

// New BPM variables
bool state = false;
bool prevState = false;
unsigned long lastCompressionTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.print("Initializing...");
  
  scale1.begin(DOUT1, CLK1, 64);  // 80SPS mode
  scale1.wait_ready_timeout(50); 
  scale1.set_scale(SCALE_FACTOR);
  scale1.tare();
  delay(500);
  lcd.clear();
}

float average(const float* arr, size_t size) {
  if (size == 0) return 0.0;
  float sum = 0;
  for (size_t i = 0; i < size; i++) {
    sum += arr[i];
  }
  return sum / size;
}

void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("D:");
  lcd.print(maxdisplacement, 1);
  lcd.print("cm W:");
  lcd.print(maxf1, 1);
  lcd.print("kg");
  
  lcd.setCursor(0, 1);
  lcd.print("BPM:");
  lcd.print(bpm);
  lcd.print(" (");
  lcd.print(count);
  lcd.print(")");
  
  if (maxdisplacement > CORRECT_DISPLACEMENT && 
      maxdisplacement < CORRECT_DISPLACEMENT + 0.5 && bpm <= maxBpm && bpm >= minBpm) {
    digitalWrite(red, LOW);
    digitalWrite(green, HIGH);
    lcd.print("GOOD");
  } else {
    digitalWrite(red, HIGH);
    digitalWrite(green, LOW);
    lcd.print("PUSH!");
  }
}

void loop() {
  //delay(10);
  // Read sensors
  potValue = analogRead(A0) + potOffset;
  f1 = scale1.get_units(1) * 4.0;  // Uncomment if using load cell

  // Track maximum values
  if (potValue > maxPotValue) maxPotValue = potValue;
  if (f1 > maxf1) maxf1 = f1;
  
  // Calculate displacement (0-6cm range)
  float newVal = float(map(potValue, 0, 290, 0, 600)) / 100.0;
    
  
  // State-based BPM detection
  state = (newVal > MIN_COMPRESSION_DEPTH);
  
  // Detect rising edge (compression start)
  if (!state && prevState) {
    count++;
    
    if (lastCompressionTime != 0) {
      unsigned long interval = millis() - lastCompressionTime;
      
      // Valid BPM range (20-300 BPM)
      if (interval > 200 && interval < 3000) {
        bpm = 60000 / interval;  // Convert ms interval to BPM
      }
    }
    lastCompressionTime = millis();
    

     maxdisplacement = float(map(maxPotValue, 0, 290, 0, 600)) / 100.0;
    
    Serial.print("Displacement: ");
    Serial.print(maxdisplacement, 3);
    Serial.print(" cm | Force: ");
    Serial.print(maxf1, 3);
    Serial.print(" kg | rate: ");
    Serial.print(bpm);
    Serial.print(" | Count: ");
    Serial.println(count);
    
    updateDisplay();
    
    // Reset trackers
    maxPotValue = 0;
    maxdisplacement=0;
    maxf1 = 0;

    // Brief LED feedback

  }
  prevState = state;

  // Periodic display update
  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL  && millis() - lastCompressionTime > 10000 ) {
    maxPotValue=0;
    maxdisplacement =0;
    Serial.println("NO Compression: ");
    Serial.print("Displacement: ");
    Serial.print(maxdisplacement, 3);
    Serial.print(" cm | Force: ");
    Serial.print(maxf1, 3);
    Serial.print(" kg \n");
    maxdisplacement=0;
    maxf1 = 0;
       lastDisplayUpdate = millis();

  }
}