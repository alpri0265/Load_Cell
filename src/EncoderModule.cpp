#include "EncoderModule.h"
#include "Config.h"

namespace {
  // Таблиця переходів квадратурного декодера: індекс = (старий_AB << 2) | новий_AB.
  // Накопичує +1/-1 за кожен "чверть-крок"; один детент EC11 = 4 чверть-кроки.
  const int8_t QDEC_TABLE[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
  };

  volatile uint8_t oldAB = 0;
  volatile int8_t  rawAccum = 0;

  void encoderISR() {
    uint8_t a = digitalRead(PIN_ENC_S1);
    uint8_t b = digitalRead(PIN_ENC_S2);
    uint8_t newAB = (a << 1) | b;
    uint8_t index = ((oldAB << 2) | newAB) & 0x0F;
    rawAccum += QDEC_TABLE[index];
    oldAB = newAB;
  }

  bool lastRawButton = HIGH;
  unsigned long lastDebounceTime = 0;
  unsigned long pressStartTime = 0;
  bool buttonPressed = false;
  bool longPressFired = false;
  bool clickPending = false;
  bool longPressPending = false;
}

void EncoderModule::begin() {
  pinMode(PIN_ENC_S1, INPUT_PULLUP);
  pinMode(PIN_ENC_S2, INPUT_PULLUP);
  pinMode(PIN_ENC_KEY, INPUT_PULLUP);

  uint8_t a = digitalRead(PIN_ENC_S1);
  uint8_t b = digitalRead(PIN_ENC_S2);
  oldAB = (a << 1) | b;

  attachInterrupt(digitalPinToInterrupt(PIN_ENC_S1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_S2), encoderISR, CHANGE);
}

int8_t EncoderModule::readStep() {
  int8_t result = 0;
  noInterrupts();
  if (rawAccum >= 4) {
    result = 1;
    rawAccum -= 4;
  } else if (rawAccum <= -4) {
    result = -1;
    rawAccum += 4;
  }
  interrupts();
  return result;
}

void EncoderModule::update() {
  bool reading = digitalRead(PIN_ENC_KEY); // LOW = натиснуто (INPUT_PULLUP)
  unsigned long now = millis();

  if (reading != lastRawButton) {
    lastDebounceTime = now;
  }

  if ((now - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
    bool pressedNow = (reading == LOW);

    if (pressedNow && !buttonPressed) {
      buttonPressed = true;
      pressStartTime = now;
      longPressFired = false;
    } else if (!pressedNow && buttonPressed) {
      buttonPressed = false;
      if (!longPressFired) {
        clickPending = true;
      }
    } else if (pressedNow && buttonPressed && !longPressFired) {
      if ((now - pressStartTime) > BUTTON_LONGPRESS_MS) {
        longPressFired = true;
        longPressPending = true;
      }
    }
  }

  lastRawButton = reading;
}

bool EncoderModule::wasClicked() {
  if (clickPending) {
    clickPending = false;
    return true;
  }
  return false;
}

bool EncoderModule::wasLongPressed() {
  if (longPressPending) {
    longPressPending = false;
    return true;
  }
  return false;
}

bool EncoderModule::isPressed() {
  return buttonPressed;
}
