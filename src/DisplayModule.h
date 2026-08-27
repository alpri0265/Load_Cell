#pragma once
#include <Arduino.h>

// Обгортка над LCD2004 (I2C): вивід рядків фіксованої ширини (доповнення пробілами).
namespace DisplayModule {
  void begin();
  void clear();
  void printLine(uint8_t row, const char *text);
}
