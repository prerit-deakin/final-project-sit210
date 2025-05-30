#define BLYNK_TEMPLATE_ID "TMPL6BMx7aROK"
#define BLYNK_TEMPLATE_NAME "Plant Monitor"
#define BLYNK_AUTH_TOKEN "tlgwxPqHdjfuMNbym8UaJHiceMd8PNNQ"

#include <Wire.h>
#include <WiFiNINA.h>
#include <BlynkSimpleWiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <DHT.h>

// --- PIN DEFINITIONS ---
#define DHTPIN 2
#define DHTTYPE DHT22
#define SOIL_PIN A0
#define PIR_PIN 7
#define BUZZER_PIN 5

// --- WiFi + IFTTT ---
char ssid[] = "WiFi-D753C7";
char pass[] = "16267541";
char iftttKey[] = "gJ_G5ib4jvxmQDljur-X_Jrhq0UYNEYkJ9SkPpo_j3s"; 

WiFiClient wifi;
HttpClient client(wifi, "maker.ifttt.com", 80);

// --- Blynk Timer ---
BlynkTimer timer;

// --- Sensor Object ---
DHT dht(DHTPIN, DHTTYPE);

// --- IFTTT Timer ---
unsigned long lastCheck = 0;
const unsigned long checkInterval = 1800000;  // 30 minutes

void sendToUno(float temp, float hum) {
  String payload = String(temp, 1) + "," + String(hum, 1);
  Wire.beginTransmission(8);  // Uno address
  Wire.write(payload.c_str());
  Wire.endTransmission();
}

void triggerIFTTT(String eventName) {
  String url = "/trigger/" + eventName + "/with/key/" + String(iftttKey);
  client.get(url);

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("IFTTT [" + eventName + "] -> Status: ");
  Serial.println(statusCode);
}

void sendSensorData() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  int soil = analogRead(SOIL_PIN);
  bool motion = digitalRead(PIR_PIN);
  unsigned long now = millis();

  // Print to Serial
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" °C | Hum: "); Serial.print(hum);
  Serial.print(" % | Soil: "); Serial.print(soil);
  Serial.print(" | Motion: "); Serial.println(motion ? "YES" : "NO");

  // --- Blynk updates ---
  Blynk.virtualWrite(V0, temp);
  Blynk.virtualWrite(V1, hum);
  Blynk.virtualWrite(V2, soil);
  Blynk.virtualWrite(V3, motion ? "Motion" : "No Motion");

  // --- I2C to Uno ---
  if (!isnan(temp) && !isnan(hum)) {
    sendToUno(temp, hum);
  }

  // --- Buzzer Logic ---
  bool alert = false;
  if (motion || (temp > 30 && !isnan(temp)) || (hum > 80 && !isnan(hum))) {
    alert = true;
  }
  digitalWrite(BUZZER_PIN, alert ? HIGH : LOW);

  // --- IFTTT Immediate Motion Alert ---
  if (motion) {
    triggerIFTTT("motion_alert");
  }

  // --- IFTTT Every 30min: Soil / Temp/Hum ---
  if (now - lastCheck > checkInterval) {
    lastCheck = now;

    if (soil > 750) {
      triggerIFTTT("soil_alert");
    }
    if ((temp > 30 || hum > 80) && !isnan(temp) && !isnan(hum)) {
      triggerIFTTT("env_alert");
    }
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();                // I2C master
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  timer.setInterval(2000L, sendSensorData);  // Every 2 seconds
}

void loop() {
  Blynk.run();
  timer.run();
}
