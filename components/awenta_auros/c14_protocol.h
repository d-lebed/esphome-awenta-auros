#pragma once

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace esphome::awenta_auros {

inline constexpr const char *TAG = "awenta_auros";
inline constexpr const char *TAG_RWT = "awenta_auros_rwt";

// C14 bus device addresses
inline constexpr uint8_t ADDR_MASTER = 41;
inline constexpr uint8_t ADDR_NANO_2 = 42;
inline constexpr uint8_t ADDR_AERO = 99;
inline constexpr uint8_t ADDR_BROADCAST_PRIMARY = 100;
inline constexpr uint8_t ADDR_BROADCAST_SECONDARY = 101;
inline constexpr uint8_t ADDR_LAN = 111;
inline constexpr uint8_t ADDR_INEXT = 112;
inline constexpr uint8_t ADDR_PC = 113;

// NANO COLOR 2 C14 register indices (R/W)
inline constexpr uint16_t REG_COMFORT_TEMP = 3;
inline constexpr uint16_t REG_ECO_WINTER_TEMP = 4;
inline constexpr uint16_t REG_ECO_COOLING_TEMP = 5;
inline constexpr uint16_t REG_MANUAL_TEMP = 6;
inline constexpr uint16_t REG_AWAY_TEMP = 7;
inline constexpr uint16_t REG_SEASON_MODE = 64;
inline constexpr uint16_t REG_FIREPLACE = 76;
inline constexpr uint16_t REG_GHE = 78;
inline constexpr uint16_t REG_BYPASS_MODE = 79;
inline constexpr uint16_t REG_FAN_MODE = 80;
inline constexpr uint16_t REG_AIRING = 81;
inline constexpr uint16_t REG_AWAY_GEAR = 84;
inline constexpr uint16_t REG_THERMOSTAT_MODE = 88;
inline constexpr uint16_t REG_HOLIDAY = 89;
inline constexpr uint16_t REG_AWAY_MODE = 98;
inline constexpr uint16_t REG_AWAY_PROGRAM = 140;
inline constexpr uint16_t REG_SUPPLY_FAN_SPEED_1 = 141;
inline constexpr uint16_t REG_SUPPLY_FAN_SPEED_2 = 142;
inline constexpr uint16_t REG_SUPPLY_FAN_SPEED_3 = 143;
inline constexpr uint16_t REG_SUPPLY_FAN_AIRING_SPEED = 144;
inline constexpr uint16_t REG_EXHAUST_FAN_SPEED_1 = 145;
inline constexpr uint16_t REG_EXHAUST_FAN_SPEED_2 = 146;
inline constexpr uint16_t REG_EXHAUST_FAN_SPEED_3 = 147;
inline constexpr uint16_t REG_EXHAUST_FAN_AIRING_SPEED = 148;
inline constexpr uint16_t REG_PREHEATER_MIN_TEMP = 161;

float decode_c14_value(uint8_t h, uint8_t l);
void encode_c14_value(float value, uint8_t &h, uint8_t &l);
uint16_t decode_c14_index(uint8_t h, uint8_t l);
void encode_c14_index(uint16_t index, uint8_t &h, uint8_t &l);
uint8_t calculate_c14_checksum(const std::vector<uint8_t> &frame);

std::string fan_mode_to_string(int32_t mode);
int32_t fan_mode_from_string(const std::string &option);
std::string thermostat_mode_to_string(int32_t mode);
int32_t thermostat_mode_from_string(const std::string &option);
std::string bypass_mode_to_string(int32_t mode);
int32_t bypass_mode_from_string(const std::string &option);

}  // namespace esphome::awenta_auros
