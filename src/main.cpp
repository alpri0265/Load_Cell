#include <Arduino.h>
#include "Config.h"
#include "EncoderModule.h"
#include "DisplayModule.h"
#include "ScaleModule.h"
#include "MenuModule.h"

void setup() {
  Serial.begin(115200);
  DisplayModule::begin();
  EncoderModule::begin();
  ScaleModule::begin();
  MenuModule::begin();
}

void loop() {
  ScaleModule::update();
  MenuModule::update();
}
