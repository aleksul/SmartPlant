#include <Wire.h>

#include <EEPROM.h>

#include "RTClib.h"
RTC_DS1307 RTC;

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

Adafruit_SSD1306 display(128, 32, &Wire);

#define SOIL_SENSOR A0
#define WATER_LEVEL_SENSOR A1
#define THERMISTOR A2
#define BUTTON1 7
#define BUTTON2 8
#define LED 5
#define PUMP 3

#define PUMP_SEC 15

#define WATER_EVERY_N_SECONDS 86400L

uint16_t minimum_water_amount = 0;
uint16_t maximum_soil_moistry_level = 512;

DateTime future;

String message = "";

void setup()
{
  pinMode(SOIL_SENSOR, INPUT);
  pinMode(WATER_LEVEL_SENSOR, INPUT);
  pinMode(THERMISTOR, INPUT);
  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(PUMP, OUTPUT);

  Wire.begin();
  if (!RTC.begin()) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    abort(); 
  }

  if (!RTC.isrunning()) {
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  DateTime now = RTC.now();
  future = DateTime(now.unixtime() + WATER_EVERY_N_SECONDS);

  EEPROM.get(0, minimum_water_amount);
  EEPROM.get(2, maximum_soil_moistry_level);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setCursor(3, 11);
  display.setTextSize(2);
  display.println("SmartPlant");
  display.display();
  delay(3000);
  display.clearDisplay();
}

void loop()
{
  checkButtons();
  DateTime now = RTC.now();
  String old_message = message;
  updateMessage(now, analogRead(THERMISTOR), analogRead(WATER_LEVEL_SENSOR), analogRead(SOIL_SENSOR));
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

void updateMessage(DateTime now, uint16_t temperature, uint16_t water_level, uint16_t soil) {
  message = String(now.day()) + "." + String(now.month()) + "." + String(now.year()) + " ";
  message += String(now.hour()) + ":" + String(now.minute()) + "\n";
  if (water_level <= minimum_water_amount) {
    message += "Water level is low\n";
  }
  if (soil < maximum_soil_moistry_level) {
    message += "Next watering\n";
    message += "on " + String(future.day()) + "." + String(future.month()) + "." + String(future.year()) + " ";
    message += String(future.hour()) + ":" + String(future.minute());
  } else {
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
  if (digitalRead(BUTTON1) == HIGH) {
    delay(10);
    if (digitalRead(BUTTON1) == HIGH) {
      EEPROM.put(0, analogRead(WATER_LEVEL_SENSOR));
      digitalWrite(LED, HIGH);
      delay(1000);
      digitalWrite(LED, LOW);
    }
  }  
  if (digitalRead(BUTTON2) == HIGH) {
    delay(10);
    if (digitalRead(BUTTON2) == HIGH) {
      EEPROM.put(2, analogRead(SOIL_SENSOR));
      digitalWrite(LED, HIGH);
      delay(1000);
      digitalWrite(LED, LOW);
    }
  }
}
