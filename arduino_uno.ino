#include <Wire.h>
#include <LiquidCrystal.h>

// LCD connected via shield (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

String incomingData = "";

void setup() {
  Serial.begin(9600);
  Wire.begin(8);  // I2C slave address
  Wire.onReceive(receiveData);

  lcd.begin(16, 2);
  lcd.print("Waiting for data");
}

void loop() {
  // Nothing in loop. updates will come via I2C
}

void receiveData(int byteCount) {
  incomingData = "";
  while (Wire.available()) {
    char c = Wire.read();
    incomingData += c;
  }

  lcd.clear();
  int commaIndex = incomingData.indexOf(',');
  if (commaIndex > 0) {
    String temp = incomingData.substring(0, commaIndex);
    String hum = incomingData.substring(commaIndex + 1);
    lcd.setCursor(0, 0);
    lcd.print("Temp: " + temp + " C");
    lcd.setCursor(0, 1);
    lcd.print("Humidity: " + hum + " %");
  } else {
    lcd.print("Error");
  }
}
