#define BLYNK_TEMPLATE_ID "TMPL6BMx7aROK"
#define BLYNK_TEMPLATE_NAME "Plant Monitor"
#define BLYNK_AUTH_TOKEN "tlgwxPqHdjfuMNbym8UaJHiceMd8PNNQ"

#include <Wire.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>
#include <DHT.h>
#include <HTTPClient.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define SOIL_PIN A0
#define PIR_PIN 7
#define BUZZER_PIN 5

char ssid[] = "WiFi-D753C7";
char pass[] = "16267541";

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

const char* motion_url = "https://maker.ifttt.com/trigger/motion_alert/with/key/gJ_G5ib4jvxmQDljur-X_Jrhq0UYNEYkJ9SkPpo_j3s";
const char* env_url    = "https://maker.ifttt.com/trigger/env_alert/with/key/gJ_G5ib4jvxmQDljur-X_Jrhq0UYNEYkJ9SkPpo_j3s";
const char* soil_url   = "https://maker.ifttt.com/trigger/soil_alert/with/key/gJ_G5ib4jvxmQDljur-X_Jrhq0UYNEYkJ9SkPpo_j3s";

// Triggered every 30 minutes for soil moisture alert, I can change it as well
void triggerSoilCheck() {
  int soil = analogRead(SOIL_PIN);
  if (soil > 750) {
    HTTPClient http;
    http.begin(soil_url);
    http.GET();
    http.end();
  }
}

void sendSensorData() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soil = analogRead(SOIL_PIN);
  bool motion = digitalRead(PIR_PIN);

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" °C | Hum: "); Serial.print(hum);
  Serial.print(" % | Soil: "); Serial.print(soil);
  Serial.print(" | Motion: "); Serial.println(motion ? "YES" : "NO");

  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V2, soil);
  Blynk.virtualWrite(V3, motion ? "Motion" : "No Motion");

  // I2C send to Uno
  if (!isnan(temp) && !isnan(hum)) {
    String data = String(temp, 1) + "," + String(hum, 1);
    Wire.beginTransmission(8);  // Address of Uno (slave)
    Wire.write(data.c_str());
    Wire.endTransmission();
  }

  // Buzzer + IFTTT alerts
  bool alert = false;
  if (motion) {
    HTTPClient http;
    http.begin(motion_url);
    http.GET();
    http.end();
    alert = true;
  }
  if (!isnan(temp) && temp > 30 || !isnan(hum) && hum > 80) {
    HTTPClient http;
    http.begin(env_url);
    http.GET();
    http.end();
    alert = true;
  }

  digitalWrite(BUZZER_PIN, alert ? HIGH : LOW);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();  // I2C master
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  timer.setInterval(2000L, sendSensorData);
  timer.setInterval(1800000L, triggerSoilCheck);  // every 30 minutes
}

void loop() {
  Blynk.run();
  timer.run();
}
