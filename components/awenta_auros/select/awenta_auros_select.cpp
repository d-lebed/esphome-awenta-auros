#include "awenta_auros_select.h"

namespace esphome::awenta_auros {

void FanModeSelect::control(size_t index) {
  this->publish_state(index);
  this->parent_->set_desired_fan_mode_index(index);
}

void ThermostatModeSelect::control(size_t index) {
  this->publish_state(index);
  this->parent_->set_desired_thermostat_mode_index(index);
}

void BypassModeSelect::control(size_t index) {
  this->publish_state(index);
  this->parent_->set_desired_bypass_mode_index(index);
}

}  // namespace esphome::awenta_auros
