#include "StorageModule.h"
#include <EEPROM.h>

namespace {
  struct StoredBlock {
    uint8_t magic;
    float   scaleFactor;
    uint8_t unit;
    uint8_t checksum;
  };

  uint8_t calcChecksum(const StoredBlock &b) {
    const uint8_t *p = reinterpret_cast<const uint8_t *>(&b);
    uint8_t sum = 0;
    for (size_t i = 0; i < sizeof(StoredBlock) - sizeof(b.checksum); i++) {
      sum ^= p[i];
    }
    return sum;
  }
}

void StorageModule::load(CalibrationData &data) {
  StoredBlock b;
  EEPROM.get(EEPROM_ADDR, b);

  if (b.magic == EEPROM_MAGIC && b.checksum == calcChecksum(b)) {
    data.scaleFactor = b.scaleFactor;
    data.unit = b.unit;
  } else {
    data.scaleFactor = DEFAULT_SCALE_FACTOR;
    data.unit = static_cast<uint8_t>(WeightUnit::KG);
  }
}

void StorageModule::save(const CalibrationData &data) {
  StoredBlock b;
  b.magic = EEPROM_MAGIC;
  b.scaleFactor = data.scaleFactor;
  b.unit = data.unit;
  b.checksum = calcChecksum(b);
  EEPROM.put(EEPROM_ADDR, b);
}
