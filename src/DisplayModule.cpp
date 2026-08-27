#include "DisplayModule.h"
#include "Config.h"
#include <LiquidCrystal_I2C.h>

namespace {
  LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS);
}

void DisplayModule::begin() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
}

void DisplayModule::clear() {
  lcd.clear();
}

void DisplayModule::printLine(uint8_t row, const char *text) {
  if (row >= LCD_ROWS) return;

  char buf[LCD_COLS + 1];
  uint8_t i = 0;
  for (; i < LCD_COLS && text[i] != '\0'; i++) {
    buf[i] = text[i];
  }
  for (; i < LCD_COLS; i++) {
    buf[i] = ' ';
  }
  buf[LCD_COLS] = '\0';

  lcd.setCursor(0, row);
  lcd.print(buf);
}
