#pragma once
#include <Arduino.h>

// Робота з тензодатчиком через HX711: фільтрація, тарування, калібрування.
namespace ScaleModule {
  void begin();
  void update();                 // неблокуюче: якщо HX711 готовий — оновлює фільтр

  float getWeightKg();           // відфільтроване значення у кг (за поточним scale factor)
  bool  isStable();

  void  tare();                  // зафіксувати поточний нуль (платформа має бути порожньою)
  void  setScaleFactor(float factor);
  float getScaleFactor();

  long  readRawValue(uint8_t samples); // блокуюче: сирі одиниці HX711 відносно тари (для калібрування)
}
