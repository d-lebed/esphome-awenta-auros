#include "awenta_auros_number.h"

namespace esphome::awenta_auros {

void ComfortTemperatureNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_temperature(REG_COMFORT_TEMP, value);
}

void EcoWinterTemperatureNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_temperature(REG_ECO_WINTER_TEMP, value);
}

void EcoCoolingTemperatureNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_temperature(REG_ECO_COOLING_TEMP, value);
}

void ManualTemperatureNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_temperature(REG_MANUAL_TEMP, value);
}

void AwayTemperatureNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_temperature(REG_AWAY_TEMP, value);
}

void PreheaterMinTemperatureNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_preheater_min_temperature(value);
}

void SupplyFanSpeed1Number::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_SPEED_1, value);
}

void SupplyFanSpeed2Number::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_SPEED_2, value);
}

void SupplyFanSpeed3Number::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_SPEED_3, value);
}

void SupplyFanAiringSpeedNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_AIRING_SPEED, value);
}

void ExhaustFanSpeed1Number::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_SPEED_1, value);
}

void ExhaustFanSpeed2Number::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_SPEED_2, value);
}

void ExhaustFanSpeed3Number::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_SPEED_3, value);
}

void ExhaustFanAiringSpeedNumber::control(float value) {
  this->publish_state(value);
  this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_AIRING_SPEED, value);
}

void SmoothFanSpeedNumber::control(float value) {
  this->parent_->set_desired_smooth_fan_speed(value);
}

}  // namespace esphome::awenta_auros
