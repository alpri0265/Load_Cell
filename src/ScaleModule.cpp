#include "ScaleModule.h"
#include "Config.h"
#include <HX711.h>
#include <math.h>

namespace {
  HX711 hx711;

  float filterBuf[WEIGHT_FILTER_SIZE];
  uint8_t filterIndex = 0;
  uint8_t filterCount = 0;
  float filteredWeight = 0.0f;

  float stableRefWeight = 0.0f;
  unsigned long stableSinceMs = 0;
  bool stableFlag = false;
}

void ScaleModule::begin() {
  hx711.begin(PIN_HX711_DOUT, PIN_HX711_SCK);
  hx711.set_scale(DEFAULT_SCALE_FACTOR);
  if (hx711.wait_ready_timeout(HX711_READY_TIMEOUT_MS, 10)) {
    hx711.tare(10);
  }
  // якщо HX711 не встиг стабілізуватись за час таймауту - не блокуємо старт
  // пристрою; тара залишиться нульовою, ScaleModule::update() підхопить
  // покази датчика пізніше, щойно він почне відповідати.
}

void ScaleModule::update() {
  if (!hx711.is_ready()) return;

  float kg = hx711.get_units(1);

  if (filterCount > 0 && fabs(kg - filteredWeight) > JUMP_RESET_THRESHOLD_KG) {
    // Різка зміна навантаження - не чекаємо, поки ковзне середнє поступово
    // "наздожене" нове значення: одразу заповнюємо ним весь буфер.
    for (uint8_t i = 0; i < WEIGHT_FILTER_SIZE; i++) filterBuf[i] = kg;
    filterIndex = 0;
    filterCount = WEIGHT_FILTER_SIZE;
    filteredWeight = kg;
  } else {
    filterBuf[filterIndex] = kg;
    filterIndex = (filterIndex + 1) % WEIGHT_FILTER_SIZE;
    if (filterCount < WEIGHT_FILTER_SIZE) filterCount++;

    float sum = 0.0f;
    for (uint8_t i = 0; i < filterCount; i++) sum += filterBuf[i];
    filteredWeight = sum / filterCount;
  }

  unsigned long now = millis();
  if (fabs(filteredWeight - stableRefWeight) > STABLE_THRESHOLD_KG) {
    stableRefWeight = filteredWeight;
    stableSinceMs = now;
    stableFlag = false;
  } else if (!stableFlag && (now - stableSinceMs) > STABLE_TIME_MS) {
    stableFlag = true;
  }
}

float ScaleModule::getWeightKg() {
  return filteredWeight;
}

bool ScaleModule::isStable() {
  return stableFlag;
}

void ScaleModule::tare() {
  if (!hx711.wait_ready_timeout(HX711_READY_TIMEOUT_MS, 10)) return;
  hx711.tare(10);
  filterIndex = 0;
  filterCount = 0;
  filteredWeight = 0.0f;
  stableFlag = false;
}

void ScaleModule::setScaleFactor(float factor) {
  hx711.set_scale(factor);
}

float ScaleModule::getScaleFactor() {
  return hx711.get_scale();
}

long ScaleModule::readRawValue(uint8_t samples) {
  if (!hx711.wait_ready_timeout(HX711_READY_TIMEOUT_MS, 10)) return 0;
  return hx711.get_value(samples);
}
