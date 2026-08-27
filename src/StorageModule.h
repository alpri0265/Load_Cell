#pragma once
#include <Arduino.h>
#include "Config.h"

struct CalibrationData {
  float   scaleFactor;
  uint8_t unit; // значення WeightUnit
};

// Збереження/зчитування калібрування у вбудованій EEPROM Arduino Nano.
namespace StorageModule {
  void load(CalibrationData &data); // якщо в EEPROM немає валідних даних — повертає значення за замовчуванням
  void save(const CalibrationData &data);
}
