#pragma once

#include "awenta_auros.h"

namespace esphome::awenta_auros {

#ifdef USE_SELECT
class FanModeSelect final : public select::Select, public Parented<AwentaAurosComponent> {
 protected:
  void control(size_t index) override {
    this->publish_state(index);
    this->parent_->set_desired_fan_mode_index(index);
  }
};

class ThermostatModeSelect final : public select::Select, public Parented<AwentaAurosComponent> {
 protected:
  void control(size_t index) override {
    this->publish_state(index);
    this->parent_->set_desired_thermostat_mode_index(index);
  }
};

class BypassModeSelect final : public select::Select, public Parented<AwentaAurosComponent> {
 protected:
  void control(size_t index) override {
    this->publish_state(index);
    this->parent_->set_desired_bypass_mode_index(index);
  }
};
#endif

#ifdef USE_SWITCH
class AiringSwitch final : public switch_::Switch, public Parented<AwentaAurosComponent> {
 protected:
  void write_state(bool state) override {
    this->publish_state(state);
    this->parent_->set_desired_airing(state);
  }
};

class FireplaceSwitch final : public switch_::Switch, public Parented<AwentaAurosComponent> {
 protected:
  void write_state(bool state) override {
    this->publish_state(state);
    this->parent_->set_desired_fireplace(state);
  }
};

class GheSwitch final : public switch_::Switch, public Parented<AwentaAurosComponent> {
 protected:
  void write_state(bool state) override {
    this->publish_state(state);
    this->parent_->set_desired_ghe(state);
  }
};
#endif

#ifdef USE_NUMBER
class ComfortTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(REG_COMFORT_TEMP, value);
  }
};

class EcoWinterTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(REG_ECO_WINTER_TEMP, value);
  }
};

class EcoCoolingTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(REG_ECO_COOLING_TEMP, value);
  }
};

class ManualTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(REG_MANUAL_TEMP, value);
  }
};

class AwayTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(REG_AWAY_TEMP, value);
  }
};

class PreheaterMinTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_preheater_min_temperature(value);
  }
};

class SupplyFanSpeed1Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_SPEED_1, value);
  }
};

class SupplyFanSpeed2Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_SPEED_2, value);
  }
};

class SupplyFanSpeed3Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_SPEED_3, value);
  }
};

class SupplyFanAiringSpeedNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_SUPPLY_FAN_AIRING_SPEED, value);
  }
};

class ExhaustFanSpeed1Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_SPEED_1, value);
  }
};

class ExhaustFanSpeed2Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_SPEED_2, value);
  }
};

class ExhaustFanSpeed3Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_SPEED_3, value);
  }
};

class ExhaustFanAiringSpeedNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_fan_speed_percent(REG_EXHAUST_FAN_AIRING_SPEED, value);
  }
};

class SmoothFanSpeedNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->parent_->set_desired_smooth_fan_speed(value);
  }
};
#endif

}  // namespace esphome::awenta_auros
