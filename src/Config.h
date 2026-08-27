#pragma once
#include <Arduino.h>

// ==================== HX711 (тензодатчик) ====================
constexpr uint8_t PIN_HX711_DOUT = 6;
constexpr uint8_t PIN_HX711_SCK  = 7;

// ==================== Енкодер EC11 (модуль 5V-KEY-S2-S1-GND) ====================
constexpr uint8_t PIN_ENC_S1  = 2;  // INT0, фаза A
constexpr uint8_t PIN_ENC_S2  = 3;  // INT1, фаза B
constexpr uint8_t PIN_ENC_KEY = 5;  // кнопка натискання

// ==================== LCD 2004 (I2C) ====================
constexpr uint8_t LCD_I2C_ADDRESS = 0x27; // якщо екран не працює — спробуйте 0x3F
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;

// ==================== Параметри ваги ====================
constexpr float   DEFAULT_SCALE_FACTOR = 1.0f;   // одиниць HX711 на 1 кг (визначається калібруванням)
constexpr uint8_t HX711_SAMPLES_AVG    = 5;       // усереднення при одному читанні
constexpr uint8_t WEIGHT_FILTER_SIZE   = 10;      // розмір ковзного середнього для відображення
constexpr float   STABLE_THRESHOLD_KG  = 0.05f;   // поріг стабільності, кг
constexpr uint16_t STABLE_TIME_MS      = 500;     // час утримання в межах порогу для статусу "стабільно"
constexpr uint16_t HX711_READY_TIMEOUT_MS = 2000; // максимум чекати готовності HX711 перед tare/калібруванням
                                                   // (бібліотека HX711 без таймауту чекає нескінченно — це б
                                                   // "вішало" весь пристрій, якщо датчик не встиг стабілізуватись)

// ==================== Кнопка енкодера ====================
constexpr uint16_t BUTTON_DEBOUNCE_MS   = 30;
constexpr uint16_t BUTTON_LONGPRESS_MS  = 800;

// ==================== EEPROM ====================
constexpr int     EEPROM_ADDR  = 0;
constexpr uint8_t EEPROM_MAGIC = 0xA5;

enum class WeightUnit : uint8_t { KG = 0, GRAM = 1 };
