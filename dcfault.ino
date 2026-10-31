#include <Wire.h>
#include <MPU6050.h>
#include <math.h>
#include <OneWire.h>
#include <DallasTemperature.h>

MPU6050 mpu;

// Motor pins
#define IN3 26
#define IN4 27

// Buzzer
#define BUZZER 13

// DS18B20
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Thresholds
float vibrationThreshold = 2.0;
float tempThreshold = 36.0;   // 🔥 set temperature limit (°C)

// Time tracking
unsigned long startTime = 0;
bool timerRunning = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  Wire.setClock(100000);

  mpu.initialize();
  delay(1000);

  sensors.begin();

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Start motor
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  Serial.println("System Started");
}

void loop() {

  // 🌡️ Read Temperature
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  // 📈 Read Vibration
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float ax_g = ax / 16384.0;
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  float vibration = sqrt(ax_g*ax_g + ay_g*ay_g + az_g*az_g);

  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("  Vib: ");
  Serial.println(vibration);

  // 🔥 TEMPERATURE FAULT (instant)
  if (temp >= tempThreshold) {
    Serial.println("OVER TEMPERATURE!");

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    digitalWrite(BUZZER, HIGH);

    while (1); // stop system
  }

  // 🔥 VIBRATION FAULT (4 sec rule)
  if (vibration > vibrationThreshold) {

    if (!timerRunning) {
      startTime = millis();
      timerRunning = true;
    }

    if (millis() - startTime >= 4000) {
      Serial.println("VIBRATION FAULT!");

      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
      digitalWrite(BUZZER, HIGH);

      while (1);
    }

  } else {
    timerRunning = false;
  }

  delay(200);
}