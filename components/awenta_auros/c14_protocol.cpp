#include "c14_protocol.h"

namespace esphome::awenta_auros {

float decode_c14_value(uint8_t h, uint8_t l) {
  int32_t value_raw = ((h & 0x7F) * 128) + (l & 0x7F) - 2000;
  return value_raw / 10.0f;
}

void encode_c14_value(float value, uint8_t &h, uint8_t &l) {
  int32_t raw_value = (int32_t) std::round(value * 10.0f) + 2000;
  if (raw_value < 0)
    raw_value = 0;
  h = (raw_value / 128) & 0x7F;
  l = (raw_value % 128) & 0x7F;
}

uint16_t decode_c14_index(uint8_t h, uint8_t l) {
  return ((h & 0x7F) * 128) + (l & 0x7F);
}

void encode_c14_index(uint16_t index, uint8_t &h, uint8_t &l) {
  h = (index / 128) & 0x7F;
  l = (index % 128) & 0x7F;
}

uint8_t calculate_c14_checksum(const std::vector<uint8_t> &frame) {
  uint32_t arithmetic_sum = 0;
  for (size_t i = 0; i < frame.size() && i < 30; i++) {
    if (i == 2)
      continue;
    arithmetic_sum += frame[i];
  }
  return arithmetic_sum & 0x7F;
}

std::string fan_mode_to_string(int32_t mode) {
  switch (mode) {
    case 0:
      return "Off";
    case 1:
      return "Speed 1";
    case 2:
      return "Speed 2";
    case 3:
      return "Speed 3";
    case 4:
      return "Party";
    case 5:
      return "Schedule";
    default:
      return "";
  }
}

int32_t fan_mode_from_string(const std::string &option) {
  if (option == "Off" || option == "0")
    return 0;
  if (option == "Speed 1" || option == "1")
    return 1;
  if (option == "Speed 2" || option == "2")
    return 2;
  if (option == "Speed 3" || option == "3")
    return 3;
  if (option == "Party" || option == "Holiday")
    return 4;
  if (option == "Schedule")
    return 5;
  return -1;
}

std::string thermostat_mode_to_string(int32_t mode) {
  switch (mode) {
    case 0:
      return "Schedule";
    case 1:
      return "Party";
    case 2:
      return "Manual";
    default:
      return "";
  }
}

int32_t thermostat_mode_from_string(const std::string &option) {
  if (option == "Schedule")
    return 0;
  if (option == "Party" || option == "Holiday")
    return 1;
  if (option == "Manual")
    return 2;
  return -1;
}

std::string bypass_mode_to_string(int32_t mode) {
  switch (mode) {
    case 0:
      return "Off";
    case 1:
      return "Auto";
    case 2:
      return "On";
    default:
      return "";
  }
}

int32_t bypass_mode_from_string(const std::string &option) {
  if (option == "Off")
    return 0;
  if (option == "Auto")
    return 1;
  if (option == "On")
    return 2;
  return -1;
}

}  // namespace esphome::awenta_auros
