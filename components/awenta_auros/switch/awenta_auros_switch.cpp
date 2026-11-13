#include "awenta_auros_switch.h"

namespace esphome::awenta_auros {

void AiringSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_desired_airing(state);
}

void FireplaceSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_desired_fireplace(state);
}

void GheSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_desired_ghe(state);
}

void AwaySwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_desired_away(state);
}

void HolidaySwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_desired_holiday(state);
}

}  // namespace esphome::awenta_auros
