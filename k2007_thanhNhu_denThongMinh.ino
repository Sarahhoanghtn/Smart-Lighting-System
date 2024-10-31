#define BLYNK_TEMPLATE_ID "TMPL6ZVRJ0kDl"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "fqquc0EwwoYRCizk60B-91FejHBe4Z8s"

#include <Arduino.h>
#include <AccelStepper.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_NeoPixel.h>
#include "Adafruit_VL53L0X.h"
//==================================================================================================
#define dw digitalWrite
#define dr digitalRead
// stepper motor pins
#define STEP1 6
#define DIR1 7
#define FULL_ROUND_STEP 800  // 360 degree

#define SERVO1_PIN 10
#define FREQ 5000

#define MQ_PIN 3

#define LED_PIN 5
#define NUM_LEDS 16
//==================================================================================================

int pos1 = 0;  //step
int pos2 = 0;  //servo
int lastPos1 = 0;
int minUs = 500, maxUs = 2500;
float tem, humi;
int mq = 0;
bool isLedOn = false;
int16_t distance;
int distances[] = {};
unsigned long lastRead = 0;
//==================================================================================================
ESP32PWM pwm;
Servo myservo1;
AccelStepper stepper1(AccelStepper::DRIVER, STEP1, DIR1);
Adafruit_AHTX0 aht;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
//=========================================================================================
BLYNK_WRITE(V0) {
  pos1 = param.asInt();
  Serial.print(pos1);
  stepper1.moveTo(pos1);
}
BLYNK_WRITE(V2) {
  pos2 = param.asInt();
  Serial.println("servo control");
  myservo1.write(pos2);
}
BLYNK_WRITE(V5) {
  isLedOn = param.asInt();
  Serial.println("led control");
}
void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  //--------------------------------------
  pinMode(STEP1, OUTPUT);
  pinMode(DIR1, OUTPUT);
  dw(STEP1, LOW);
  dw(DIR1, LOW);
  //--------------------------------------
  stepper1.setMaxSpeed(1200);
  stepper1.setAcceleration(200);  // Acceleration
  stepper1.setSpeed(1000);         // Set initial speed


  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  myservo1.setPeriodHertz(50);  // Standard 50hz servo
  myservo1.attach(SERVO1_PIN, minUs, maxUs);

  aht.begin();
  lox.begin();

  // pinMode(MQ_PIN, INPUT);

  strip.begin();
  strip.show();

  WiFiManager wm;
  bool res;
  res = wm.autoConnect("denThongMinh", "66668888");  // password protected ap

  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());

  // scanAtStartup();
  // int smallestDistanceOfStepperAngle = findMinIndex(distances, sizeof(distances) / sizeof(distances[0])) * 5;
  // stepper1.moveTo(smallestDistanceOfStepperAngle);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  stepper1.run();  // Non-blocking run
  stepper1.moveTo(pos1);

  if (isLedOn == true) {
    colorWipe(strip.Color(255, 255, 255), 50);
  } else {
    turnOff();
  }

  if (millis() - lastRead > 2000) {
    Serial.println("sending data");
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    tem = temp.temperature;
    humi = humidity.relative_humidity;
    Blynk.virtualWrite(V3, tem);
    Blynk.virtualWrite(V1, humi);
    Blynk.virtualWrite(V4, analogRead(MQ_PIN));
    Blynk.virtualWrite(V6, readDistance());
    lastRead = millis();
  }
}

int16_t readDistance() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  return measure.RangeMilliMeter;
}

void colorWipe(uint32_t color, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}
void turnOff() {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0);  // Set each pixel to 'off' (black)
  }
  strip.show();  // Update the strip to apply changes
}

void scanAtStartup() {
  myservo1.write(90);
  for (int i = 0; i < 72; i++) {
    // stepper1.moveTo(i * 5);
    delay(300);
    distances[i] = readDistance();
  }
}

// void bubbleSort(int arr[], int n) {
//   for (int i = 0; i < n - 1; i++) {
//     for (int j = 0; j < n - i - 1; j++) {
//       if (arr[j] > arr[j + 1]) {
//         Swap arr[j] and arr[j+1]
//         int temp = arr[j];
//         arr[j] = arr[j + 1];
//         arr[j + 1] = temp;
//       }
//     }
//   }
// }


int findMinIndex(int arr[], int n) {
  int minIndex = 0;  // Start with the first element

  for (int i = 1; i < n; i++) {
    if (arr[i] < arr[minIndex]) {
      minIndex = i;  // Update the index if a smaller number is found
    }
  }

  return minIndex;
}