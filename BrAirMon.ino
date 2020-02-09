// (C) 2020 - Toby Gray <toby.gray@gmail.com>

#include <LiquidCrystal.h>
// Needs https://github.com/adafruit/DHT-sensor-library and https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <SoftwareSerial.h>
//LCD pin to Arduino
const int PIN_RESET = 8;
const int PIN_ENABLED = 9;
const int PIN_DATA_1 = 4;
const int PIN_DATA_2 = 5;
const int PIN_DATA_3 = 6;
const int PIN_DATA_4 = 7;
const int PIN_BACKLIGHT = 10;
const int PIN_RX = A1;
const int PIN_TX = A2;
const int PIN_CALIBRATE = 2;
const int PIN_DHT = 3;
#define DHTTYPE DHT22
// CO2 sensor takes 2 minutes to calibrate.
const int STARTUP_TIME_S = 120;

byte EMPTY_CHAR_DATA[] {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
};
byte FILLED_CHAR_DATA[] {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};
byte DEGREE_CHAR_DATA[] {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000,
};

const int DISPLAY_WIDTH = 16;
const int DISPLAY_HEIGHT = 2;
const uint8_t EMPTY_CHAR = 0;
const uint8_t FILLED_CHAR = 1;
const uint8_t DEGREE_CHAR = 2;

const char* BOOT_PHRASES[] = {
  "Pre-heating     ",
  "Ionizing Gates  ",
  "Inverting Matrix",
  "Counting Atoms  ",
  "Blending Splines",
  "Shifting Phase  ",
  "Warming Polarity",
  "Calibrating Ptz.",
  "Tweaking IR bulb",
  "Checking Cloud  ",
  "   -- Done --   ",
};

LiquidCrystal lcd(PIN_RESET,  PIN_ENABLED,  PIN_DATA_1,  PIN_DATA_2,  PIN_DATA_3,  PIN_DATA_4);

DHT dht(PIN_DHT, DHTTYPE);
SoftwareSerial sensor(PIN_RX, PIN_TX);

const byte requestReading[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte result[9];
long lastSampleTime = 0;

void setup() {
  Serial.begin(9600);
  sensor.begin(9600);
  lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  lcd.createChar(EMPTY_CHAR, EMPTY_CHAR_DATA);
  lcd.createChar(FILLED_CHAR, FILLED_CHAR_DATA);
  lcd.createChar(DEGREE_CHAR, DEGREE_CHAR_DATA);
  dht.begin();
  pinMode(PIN_CALIBRATE, OUTPUT);
  digitalWrite(PIN_CALIBRATE, HIGH);
}

void pad_value(int value, int digits) {
  int digit_count = 1;
  while (value > 9) {
    value /= 10;
    digit_count += 1;
  }
  while (digit_count < digits) {
    digit_count += 1;
    lcd.write(" ");
  }
}

bool isSelectButton(int x) {
  return x > 600 && x < 800;
}

int drawBoot() {
  int offset = millis() / (STARTUP_TIME_S * 100);
  for (int i = 0; i < 10; ++i) {
    lcd.setCursor(i, 0);
    if (i >= offset) {
      lcd.write(EMPTY_CHAR);
    } else {
      lcd.write(FILLED_CHAR);
    }
  }
  lcd.setCursor(11, 0);
  int percent = millis() / (STARTUP_TIME_S * 10);
  int done = 0;
  if (percent > 100) {
    percent = 100;
    done = 1;
  }
  pad_value(percent, 3);
  lcd.print(percent);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print(BOOT_PHRASES[offset]);
  int x;
  x = analogRead(0);
  if (isSelectButton(x)) {
    // Select pressed, skip boot.
    done = true;
  }
  return done;
}

void display_values(int temp, int humidity, int carbon_dioxide) {
  lcd.setCursor(0, 0);
  pad_value(temp, 3);
  lcd.print(temp);
  lcd.write(DEGREE_CHAR);
  lcd.print("C");
  lcd.setCursor(8, 0);
  pad_value(humidity, 3);
  lcd.print(humidity);
  lcd.print("%rh");
  lcd.setCursor(0, 1);
  pad_value(carbon_dioxide, 4);
  lcd.print(carbon_dioxide);
  lcd.print("ppm CO2");
}

int readPPMSerial() {
  for (int i = 0; i < 9; i++) {
    sensor.write(requestReading[i]);
  }
  while (sensor.available() < 9) {}; // wait for response
  for (int i = 0; i < 9; i++) {
    result[i] = sensor.read();
  }
  int high = result[2];
  int low = result[3];
  return high * 256 + low;
}

void loop() {
  delay(50);
  static int boot_done = 0;
  static unsigned long next_serial = 0;

  if (!boot_done) {
    boot_done = drawBoot();
    if (boot_done) {
      delay(1000);
      lcd.clear();
    }
    return;
  }
  float humidity_float = dht.readHumidity();
  float temp_float = dht.readTemperature();

  int temp = temp_float;
  int humidity = humidity_float;
  int carbon_dioxide = readPPMSerial();

  display_values(temp, humidity, carbon_dioxide);
  if (millis() > next_serial) {
    Serial.print(millis());
    Serial.print(",");
    Serial.print(temp_float);
    Serial.print(",");
    Serial.print(humidity_float);
    Serial.print(",");
    Serial.print(carbon_dioxide);
    Serial.print("\n");
    next_serial += 1000;
  }
} 
