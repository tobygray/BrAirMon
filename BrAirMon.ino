// (C) 2020 - Toby Gray <toby.gray@gmail.com>
#include <LiquidCrystal.h>
//LCD pin to Arduino
const int PIN_RESET = 8;
const int PIN_ENABLED = 9;
const int PIN_DATA_1 = 4;
const int PIN_DATA_2 = 5;
const int PIN_DATA_3 = 6;
const int PIN_DATA_4 = 7;
const int PIN_BACKLIGHT = 10;
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

const int DISPLAY_WIDTH = 16;
const int DISPLAY_HEIGHT = 2;
const uint8_t EMPTY_CHAR = 0;
const uint8_t FILLED_CHAR = 1;

LiquidCrystal lcd(PIN_RESET,  PIN_ENABLED,  PIN_DATA_1,  PIN_DATA_2,  PIN_DATA_3,  PIN_DATA_4);

void setup() {
 lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
 lcd.createChar(EMPTY_CHAR, EMPTY_CHAR_DATA);
 lcd.createChar(FILLED_CHAR, FILLED_CHAR_DATA); 
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
  lcd.print(percent);
  lcd.print("%");
  return done;
}

void loop() {
  static int boot_done = 0;
  if (!boot_done) {
    boot_done = drawBoot();
    if (boot_done) {
      delay(1000);
      lcd.clear();
    }
    return;
  }

  int x;
  x = analogRead (0);
  lcd.setCursor(10,1);
  if (x < 60) {
    lcd.print (" Right");
  }
  else if (x < 200) {
    lcd.print ("    Up");
  }
  else if (x < 400){
    lcd.print ("  Down");
  }
  else if (x < 600){
    lcd.print ("  Left");
  }
  else if (x < 800){
    lcd.print ("Select");
  }
} 
