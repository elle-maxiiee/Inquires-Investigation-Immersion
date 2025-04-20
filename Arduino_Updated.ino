#include <Stepper.h>
#include <Servo.h>

// === Constants === //
const int stepsPerRevolution = 2048; // Full rotation for 28BYJ-48 motor

// === Pin Setup === //
const int rainSensorPin = A0;      // Raindrop sensor (Analog)
const int proxSensorPin = 6;       // Proximity switch (Digital)
const int servoPin = 7;            // Servo SG90 control pin

// Stepper motor pins via ULN2003
const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

// === Objects === //
Servo myServo;
Stepper stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);

// === Setup === //
void setup() {
  pinMode(rainSensorPin, INPUT);
  pinMode(proxSensorPin, INPUT);

  myServo.attach(servoPin);
  myServo.write(0); // Default to 0 degrees

  stepper.setSpeed(10); // RPM, adjust if needed

  Serial.begin(9600);
}

// === Loop === //
void loop() {
  int rainValue = analogRead(rainSensorPin);
  bool proximityDetected = digitalRead(proxSensorPin) == HIGH;

  Serial.print("Rain Sensor Value: ");
  Serial.println(rainValue);

  if (proximityDetected) {
    delay(100); // Small debounce delay

    if (rainValue <= 300) {
      Serial.println("Wet object detected!");

      // Stepper rotates 180° to the RIGHT
      stepper.step(stepsPerRevolution / 2);

      // Servo opens like a door to the LEFT (90°)
      myServo.write(0); // Closed position
      delay(500);
      myServo.write(90); // Open left
      delay(1000);
      myServo.write(0); // Close again
      delay(500);

      // Stepper auto-resets back 180° LEFT
      stepper.step(-stepsPerRevolution / 2);
    }
    else {
      Serial.println("Dry object detected!");

      // Stepper rotates 180° to the LEFT
      stepper.step(-stepsPerRevolution / 2);

      // Servo opens like a door to the RIGHT (90°)
      myServo.write(0); // Closed position
      delay(500);
      myServo.write(90); // Open right
      delay(1000);
      myServo.write(0); // Close again
      delay(500);

      // Stepper auto-resets back 180° RIGHT
      stepper.step(stepsPerRevolution / 2);
    }
  }

  delay(200); // Delay for sensor stability
}