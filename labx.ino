#include <LiquidCrystal.h>
#include <Servo.h>
#include <dht.h> //library to control dht11 sensor

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int pinDHT10 = 10; // pin 10 connected to DHt11 senor data

float tempC = 0;
float tempF = 0;
float hum = 0;
int chk;

unsigned long lastDHTRead = 0;
dht DHT; // CREATE SENSOR

// toggel
bool toggle = false;

// Ultrasonic
const int trigPin = 8;
const int echoPin = 9;

// Actuators
const int servoPin = 6;
const int fanPin = 7;

// Joystick
const int joyYPin = A1;
const int joyButtonPin = 18;   // SW must be on pin 18 for interrupt

Servo turretServo;

volatile bool fanOverrideOff = false;
volatile unsigned long lastPressTime = 0;

int servoAngle = 90;
float distanceCM = 999;

int fanPWM = 0;
String fanText = "OFF";

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(fanPin, OUTPUT);
  analogWrite(fanPin, 0);

  pinMode(joyButtonPin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(joyButtonPin), turnFanOff, FALLING);

  turretServo.attach(servoPin);
  turretServo.write(90);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Fan Turret");
  delay(1000);
  lcd.clear();
}

void loop() {
  controlServo();

  distanceCM = readUltrasonic();

  updateFan();
  updateLCD();


if (millis() - lastDHTRead >= 2000) {
  lastDHTRead = millis();

  chk = DHT.read11(pinDHT10);

  if (chk == DHTLIB_OK) {
    tempC = DHT.temperature;
    tempF = (tempC * 9.0 / 5.0) + 32.0;
    hum = DHT.humidity;
  }
}
  lcd.print(" ");
  lcd.print(tempF);
  lcd.print("F");

  delay(120);
}

void controlServo() {
  int joyY = analogRead(joyYPin);
  int targetAngle = map(joyY, 0, 1023, 0, 180);

  if (abs(targetAngle - servoAngle) > 2) {
    servoAngle = targetAngle;
    turretServo.write(servoAngle);
  }
}

float readUltrasonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) return 999;

  float distance = duration / 58.0;

  if (distance < 2 || distance > 300) return 999;

  return distance;
}

void updateFan() {
  // Object is not close: fan off and reset override
  if (distanceCM == 999 || distanceCM > 30) {
    fanOverrideOff = false;
    fanPWM = 0;
    fanText = "OFF";
  }

  // Object is close: fan on unless joystick interrupt turned it off
  else {
    if (fanOverrideOff) {
      fanPWM = 0;
      fanText = "MAN OFF";
    } else {
      fanPWM = 255;
      fanText = "ON";
    }
  }

  analogWrite(fanPin, fanPWM);
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);

  lcd.print("D:");
  if (distanceCM == 999) {
    lcd.print("---");
  } else {
    lcd.print((int)distanceCM);
    lcd.print("cm");
  }

  lcd.print(" A:");
  lcd.print(servoAngle);

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);

  lcd.print("Fan:");
  lcd.print(fanText);
}

void turnFanOff() {
  unsigned long now = millis();

if(toggle){
  if (now - lastPressTime > 250) {
    fanOverrideOff = true;
    toggle = false;
    lastPressTime = now;
  }
}

if(!toggle){
  if (now - lastPressTime > 250) {
    fanOverrideOff = false;
    toggle = true;
    lastPressTime = now;
  }
}
}