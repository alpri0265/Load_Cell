#include "MenuModule.h"
#include "Config.h"
#include "EncoderModule.h"
#include "DisplayModule.h"
#include "ScaleModule.h"
#include "StorageModule.h"
#include <stdio.h>

// Текст інтерфейсу навмисно англійською: стандартний контролер HD44780
// (більшість модулів LCD2004) має вбудований ROM-шрифт лише з ASCII +
// японською катаканою, кирилиці в ньому немає — україномовний текст
// відображався б спотвореними символами.
namespace {
  enum class UiState : uint8_t {
    MAIN,
    MENU,
    CALIB_TARE,
    CALIB_PLACE_WEIGHT,
    CALIB_ENTER_VALUE,
    CALIB_DONE,
    UNIT_SELECT
  };

  const char *const MENU_ITEMS[] = { "Tare", "Calibrate", "Units", "Exit" };
  constexpr uint8_t MENU_ITEMS_COUNT = 4; // = LCD_ROWS: кожен пункт займає свій рядок

  UiState state = UiState::MAIN;
  uint8_t menuIndex = 0;
  bool dirty = true;
  unsigned long stateEnterTime = 0;
  unsigned long lastMainRefresh = 0;

  CalibrationData calib;
  WeightUnit currentUnit = WeightUnit::KG;

  float calibKnownWeightKg = 100.0f;
  long  calibRawValue = 0;

  void enterState(UiState s) {
    state = s;
    stateEnterTime = millis();
    dirty = true;
  }

  float toDisplayUnit(float kg) {
    return (currentUnit == WeightUnit::GRAM) ? kg * 1000.0f : kg;
  }

  const char *unitSuffix() {
    return (currentUnit == WeightUnit::GRAM) ? "g" : "kg";
  }

  void renderMain() {
    char buf[21];
    char numBuf[10];
    DisplayModule::printLine(0, "      LOAD CELL");
    float w = toDisplayUnit(ScaleModule::getWeightKg());
    dtostrf(w, 1, 1, numBuf);
    snprintf(buf, sizeof(buf), "%s %-3s %s", numBuf, unitSuffix(),
             ScaleModule::isStable() ? "[OK]" : "[..]");
    DisplayModule::printLine(1, buf);
    DisplayModule::printLine(2, "");
    DisplayModule::printLine(3, "Hold=Tare Clk=Menu");
  }

  void renderMenu() {
    char buf[21];
    for (uint8_t i = 0; i < MENU_ITEMS_COUNT; i++) {
      snprintf(buf, sizeof(buf), "%s%s", (i == menuIndex ? "> " : "  "), MENU_ITEMS[i]);
      DisplayModule::printLine(i, buf);
    }
  }

  void renderCalibTare() {
    DisplayModule::printLine(0, "Calibrate step 1/2");
    DisplayModule::printLine(1, "Remove all weight");
    DisplayModule::printLine(2, "from platform");
    DisplayModule::printLine(3, "Click = next");
  }

  void renderCalibPlaceWeight() {
    DisplayModule::printLine(0, "Calibrate step 2/2");
    DisplayModule::printLine(1, "Place known weight");
    DisplayModule::printLine(2, "on platform");
    DisplayModule::printLine(3, "Click = next");
  }

  float calibStepSize() {
    if (calibKnownWeightKg < 10.0f) return 0.1f;
    if (calibKnownWeightKg < 100.0f) return 1.0f;
    if (calibKnownWeightKg < 500.0f) return 10.0f;
    return 50.0f;
  }

  void renderCalibEnterValue() {
    char buf[21];
    char numBuf[10];
    DisplayModule::printLine(0, "Enter ref. weight");
    dtostrf(calibKnownWeightKg, 1, 1, numBuf);
    snprintf(buf, sizeof(buf), "   >> %s kg <<", numBuf);
    DisplayModule::printLine(1, buf);
    dtostrf(calibStepSize(), 1, 1, numBuf);
    snprintf(buf, sizeof(buf), "Turn: +/- %s kg", numBuf);
    DisplayModule::printLine(2, buf);
    DisplayModule::printLine(3, "Click = save");
  }

  void renderCalibDone() {
    DisplayModule::printLine(0, "");
    DisplayModule::printLine(1, "   Calibration");
    DisplayModule::printLine(2, "     saved!");
    DisplayModule::printLine(3, "");
  }

  void renderUnitSelect() {
    char buf[21];
    DisplayModule::printLine(0, "Units");
    snprintf(buf, sizeof(buf), "%s kg", (currentUnit == WeightUnit::KG ? ">" : " "));
    DisplayModule::printLine(1, buf);
    snprintf(buf, sizeof(buf), "%s g", (currentUnit == WeightUnit::GRAM ? ">" : " "));
    DisplayModule::printLine(2, buf);
    DisplayModule::printLine(3, "Click = OK");
  }

  void saveCalibration() {
    calib.scaleFactor = ScaleModule::getScaleFactor();
    calib.unit = static_cast<uint8_t>(currentUnit);
    StorageModule::save(calib);
  }
}

void MenuModule::begin() {
  StorageModule::load(calib);
  currentUnit = static_cast<WeightUnit>(calib.unit);
  ScaleModule::setScaleFactor(calib.scaleFactor);
  enterState(UiState::MAIN);
}

void MenuModule::update() {
  EncoderModule::update();
  int8_t step = EncoderModule::readStep();
  bool clicked = EncoderModule::wasClicked();
  bool longPressed = EncoderModule::wasLongPressed();

  switch (state) {
    case UiState::MAIN: {
      if (longPressed) {
        ScaleModule::tare();
      }
      if (clicked) {
        menuIndex = 0;
        enterState(UiState::MENU);
      }
      unsigned long now = millis();
      if (now - lastMainRefresh > 200) {
        lastMainRefresh = now;
        renderMain();
      }
      break;
    }

    case UiState::MENU: {
      if (step != 0) {
        int8_t next = static_cast<int8_t>(menuIndex) + step;
        if (next < 0) next = MENU_ITEMS_COUNT - 1;
        if (next >= MENU_ITEMS_COUNT) next = 0;
        menuIndex = static_cast<uint8_t>(next);
        dirty = true;
      }
      if (longPressed) {
        enterState(UiState::MAIN);
      } else if (clicked) {
        switch (menuIndex) {
          case 0: // Tare
            ScaleModule::tare();
            enterState(UiState::MAIN);
            break;
          case 1: // Calibrate
            enterState(UiState::CALIB_TARE);
            break;
          case 2: // Units
            enterState(UiState::UNIT_SELECT);
            break;
          default: // Exit
            enterState(UiState::MAIN);
            break;
        }
      }
      break;
    }

    case UiState::CALIB_TARE: {
      if (longPressed) {
        enterState(UiState::MAIN);
      } else if (clicked) {
        ScaleModule::tare();
        enterState(UiState::CALIB_PLACE_WEIGHT);
      }
      break;
    }

    case UiState::CALIB_PLACE_WEIGHT: {
      if (longPressed) {
        enterState(UiState::MAIN);
      } else if (clicked) {
        calibRawValue = ScaleModule::readRawValue(HX711_SAMPLES_AVG);
        if (calibRawValue != 0) {
          enterState(UiState::CALIB_ENTER_VALUE);
        }
        // якщо raw == 0 — ймовірно вагу ще не покладено, лишаємось на цьому екрані
      }
      break;
    }

    case UiState::CALIB_ENTER_VALUE: {
      if (step != 0) {
        calibKnownWeightKg += step * 0.5f;
        if (calibKnownWeightKg < 0.1f) calibKnownWeightKg = 0.1f;
        if (calibKnownWeightKg > 2000.0f) calibKnownWeightKg = 2000.0f;
        dirty = true;
      }
      if (longPressed) {
        enterState(UiState::MAIN);
      } else if (clicked) {
        float factor = static_cast<float>(calibRawValue) / calibKnownWeightKg;
        ScaleModule::setScaleFactor(factor);
        saveCalibration();
        enterState(UiState::CALIB_DONE);
      }
      break;
    }

    case UiState::CALIB_DONE: {
      if (millis() - stateEnterTime > 2000) {
        enterState(UiState::MAIN);
      }
      break;
    }

    case UiState::UNIT_SELECT: {
      if (step != 0) {
        currentUnit = (currentUnit == WeightUnit::KG) ? WeightUnit::GRAM : WeightUnit::KG;
        dirty = true;
      }
      if (longPressed) {
        enterState(UiState::MENU);
      } else if (clicked) {
        saveCalibration();
        menuIndex = 2;
        enterState(UiState::MENU);
      }
      break;
    }
  }

  if (state != UiState::MAIN && dirty) {
    switch (state) {
      case UiState::MENU:               renderMenu(); break;
      case UiState::CALIB_TARE:         renderCalibTare(); break;
      case UiState::CALIB_PLACE_WEIGHT: renderCalibPlaceWeight(); break;
      case UiState::CALIB_ENTER_VALUE:  renderCalibEnterValue(); break;
      case UiState::CALIB_DONE:         renderCalibDone(); break;
      case UiState::UNIT_SELECT:        renderUnitSelect(); break;
      default: break;
    }
    dirty = false;
  }
}
