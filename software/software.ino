#include <Wire.h>

#include <EEPROM.h>

#include "RTClib.h"
RTC_DS1307 RTC;

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

Adafruit_SSD1306 display(128, 32, &Wire, -1);

#define SOIL_SENSOR A0
#define THERMISTOR A2
#define BUTTON1 7
#define BUTTON2 8
#define LED 5
#define PUMP 3

#define PUMP_SEC 15

#define WATER_EVERY_N_SECONDS 259200L

uint16_t maximum_soil_moistry_level = 512;

DateTime future;

String message = "";

void setup()
{
  pinMode(SOIL_SENSOR, INPUT);
  pinMode(THERMISTOR, INPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(PUMP, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    digitalWrite(LED_BUILTIN, HIGH);
    abort(); 
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(3, 11);
  display.setTextSize(2);
  display.println("SmartPlant");
  display.display();
  delay(3000);
  display.clearDisplay();

  if (!RTC.begin()) {
    digitalWrite(LED_BUILTIN, HIGH);
    abort(); 
  }

  if (! RTC.isrunning()) {
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  DateTime now = RTC.now();
  future = DateTime(now.unixtime() + WATER_EVERY_N_SECONDS);

  EEPROM.get(2, maximum_soil_moistry_level);

  
}

void loop()
{
  checkButtons();
  DateTime now = RTC.now();
  String old_message = message;
  updateMessage(now, analogRead(SOIL_SENSOR));
  if (old_message != message) {
    updateDisplay();
  }
  if (now.unixtime() >= future.unixtime() && analogRead(SOIL_SENSOR) < maximum_soil_moistry_level) {
    future = DateTime(now.unixtime() + WATER_EVERY_N_SECONDS);
    digitalWrite(PUMP, HIGH);
    delay(PUMP_SEC);
    digitalWrite(PUMP, LOW);
  }
  delay(10);
}

void updateMessage(DateTime now, uint16_t soil) {
  message = "Next watering\n";
  message += "in " + String(future.unixtime() - now.unixtime()) + "s\n";
  if (soil >= maximum_soil_moistry_level) {
    message += "Soil is moist enough";
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
}

void checkButtons() {
  if (digitalRead(BUTTON2) == HIGH) {
    delay(10);
    if (digitalRead(BUTTON2) == HIGH) {
      maximum_soil_moistry_level = analogRead(SOIL_SENSOR) + 10;
      EEPROM.put(2, maximum_soil_moistry_level);
      digitalWrite(LED, HIGH);
      delay(1000);
      digitalWrite(LED, LOW);
    }
  }
}
