#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>
#include <Servo.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Stepper motor setup
const int stepsPerRevolution = 2048;
Stepper stepper(stepsPerRevolution, 8, 10, 9, 11);

// Servo motor
Servo myServo;

// Sensor pins
const int raindropPin = A0;
const int proximityPin = 6;
const int servoPin = 7;

// Function Prototypes
void showStandbyMessage();
void displayWasteType(String line1, String line2);
void displayMoisture(String type, int value);
void processWaste();
void countdownAndReset();

bool hasRecentWaste = false;
bool isDryLooping = false;
unsigned long lastActionTime = 0;
const unsigned long dryWasteTimeout = 5000; // 5 seconds

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(proximityPin, INPUT);
  stepper.setSpeed(15);
  myServo.attach(servoPin);
  myServo.write(90); // Close position

  showStandbyMessage();
}

void loop() {
  int rainValue = analogRead(raindropPin);
  int metalSignal = digitalRead(proximityPin);
  bool isMetalDetected = (metalSignal == HIGH);
  unsigned long currentTime = millis();

  Serial.print("Rain Value: ");
  Serial.println(rainValue);
  Serial.print("Metal Detected: ");
  Serial.println(isMetalDetected);

  if (rainValue >= 200 && rainValue < 600) {
    hasRecentWaste = true;
    isDryLooping = false;
    displayMoisture("WET WASTE", rainValue);
    processWaste();
    countdownAndReset();
    lastActionTime = millis();
  }
  else if (rainValue >= 600 && !isMetalDetected) {
    if (!isDryLooping) {
      hasRecentWaste = true;
      isDryLooping = true;
      displayWasteType("TYPE:", "DRY WASTE");
      delay(5000);
      showStandbyMessage();
      lastActionTime = millis();
    }
  }
  else if (rainValue >= 600 && isMetalDetected) {
    // Confirm metal detection with a delay and recheck to prevent false positives
    delay(200);
    metalSignal = digitalRead(proximityPin);
    isMetalDetected = (metalSignal == HIGH);

    if (isMetalDetected) {
      hasRecentWaste = true;
      isDryLooping = false;
      displayWasteType("TYPE:", "METAL WASTE");
      processWaste();
      countdownAndReset();
      lastActionTime = millis();
    } else {
      showStandbyMessage();
    }
  }
  else {
    hasRecentWaste = false;
    isDryLooping = false;
    showStandbyMessage();
  }

  delay(1000);
}

void displayWasteType(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void displayMoisture(String type, int value) {
  int moisturePercent = map(value, 0, 1023, 100, 0);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TYPE: " + type);
  lcd.setCursor(0, 1);
  lcd.print("MOISTURE: ");
  lcd.print(moisturePercent);
  lcd.print("%");
}

void showStandbyMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BIN-SENSE READY");
  lcd.setCursor(0, 1);
  lcd.print("TO SEGREGATE");
}

void processWaste() {
  int steps120deg = stepsPerRevolution * 120 / 360;
  stepper.step(steps120deg);
  delay(500);

  myServo.write(0);
  delay(1000);

  myServo.write(90);
  delay(500);

  stepper.step(-steps120deg);
  delay(500);
}

void countdownAndReset() {
  for (int i = 5; i >= 1; i--) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("NEXT CYCLE IN:");
    lcd.setCursor(0, 1);
    lcd.print(i);
    delay(1000);
  }
  showStandbyMessage();
}