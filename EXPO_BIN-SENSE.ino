#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <Servo.h>

// --- LCD Setup ---
LiquidCrystal_I2C lcd(0x27, 16, 2); // basta mga GND emerut 

// --- Stepper Setup ---
const int stepsPerRevolution = 2048;
const int step120deg = stepsPerRevolution / 3; // ~682 steps (sabi ni Meta kay Em)
Stepper stepper(stepsPerRevolution, 8, 10, 9, 11);

// --- Servo Setup ---
Servo servo;

// --- Pins ---
const int rainSensorPin = A0;
const int proximityPin = 6;
const int servoPin = 7;

// --- Thresholds ---
const int wetThreshold = 381;
const int dryValue = 1023;

// --- Rotation Tracking ---
int currentPosition = 0;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(proximityPin, INPUT);
  servo.attach(servoPin);
  servo.write(0); // Close servo

  stepper.setSpeed(10); // RPM yug speed
}

void loop() {
  int rainValue = analogRead(rainSensorPin);
  bool metalDetected = digitalRead(proximityPin) == LOW;

  Serial.print("Rain: ");
  Serial.print(rainValue);
  Serial.print(" | Metal: ");
  Serial.println(metalDetected);

  if (rainValue >= wetThreshold) {
    processWaste("WET", "WASTE", 0);
  } else if (rainValue == dryValue && metalDetected) {
    processWaste("METAL", "WASTE", 2);
  } else if (rainValue == dryValue && !metalDetected) {
    processWaste("DRY", "WASTE", 1);
  } else {
    // Show "Bin-Sense Ready!" pang standby mode
    lcd.clear();
    lcd.setCursor(3, 0); // center "Bin-Sense"
    lcd.print("Bin-Sense");
    lcd.setCursor(5, 1); // center "Ready!"
    lcd.print("Ready!");
  }

  delay(1000); // Check again after 1 second
}

void processWaste(String line1, String line2, int targetPosition) {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print(line1);
  lcd.setCursor(4, 1);
  lcd.print(line2);

  int stepDiff = (targetPosition - currentPosition + 3) % 3;
  int stepsToMove = stepDiff * step120deg;

  stepper.step(stepsToMove);
  currentPosition = targetPosition;

  delay(500);

  servo.write(90); // Open
  delay(1000);

  servo.write(0); // Close
  delay(500);
}