#include "awenta_auros_climate.h"

#ifdef USE_CLIMATE
#include "awenta_auros.h"

namespace esphome::awenta_auros {

AwentaAurosClimate::AwentaAurosClimate() = default;

void AwentaAurosClimate::apply_bus_fan_mode(int32_t fs) {
  switch (fs) {
    case 0:
      this->set_fan_mode_(climate::CLIMATE_FAN_OFF);
      break;
    case 1:
      this->set_fan_mode_(climate::CLIMATE_FAN_LOW);
      break;
    case 2:
      this->set_fan_mode_(climate::CLIMATE_FAN_MEDIUM);
      break;
    case 3:
      this->set_fan_mode_(climate::CLIMATE_FAN_HIGH);
      break;
    case 4:
      this->set_fan_mode_(climate::CLIMATE_FAN_DIFFUSE);  // Party
      break;
    case 5:
      this->set_fan_mode_(climate::CLIMATE_FAN_AUTO);  // Schedule
      break;
  }
}

void AwentaAurosClimate::apply_bus_preset(int32_t therm) {
  switch (therm) {
    case 0:
      this->set_preset_(climate::CLIMATE_PRESET_ECO);  // Schedule
      break;
    case 1:
      this->set_preset_(climate::CLIMATE_PRESET_BOOST);  // Party
      break;
    case 2:
      this->set_preset_(climate::CLIMATE_PRESET_HOME);  // Manual
      break;
  }
}

climate::ClimateTraits AwentaAurosClimate::traits() {
  auto traits = climate::ClimateTraits();
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE |
                           climate::CLIMATE_SUPPORTS_CURRENT_HUMIDITY |
                           climate::CLIMATE_SUPPORTS_ACTION);
  traits.set_supported_modes({
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_HEAT_COOL,
      climate::CLIMATE_MODE_FAN_ONLY,
  });
  traits.set_supported_fan_modes({
      climate::CLIMATE_FAN_OFF,
      climate::CLIMATE_FAN_LOW,
      climate::CLIMATE_FAN_MEDIUM,
      climate::CLIMATE_FAN_HIGH,
      climate::CLIMATE_FAN_DIFFUSE,
      climate::CLIMATE_FAN_AUTO,
  });
  traits.set_supported_presets({
      climate::CLIMATE_PRESET_ECO,
      climate::CLIMATE_PRESET_BOOST,
      climate::CLIMATE_PRESET_HOME,
  });
  traits.set_visual_min_temperature(10.0f);
  traits.set_visual_max_temperature(30.0f);
  traits.set_visual_target_temperature_step(0.1f);
  traits.set_visual_current_temperature_step(0.1f);
  return traits;
}

void AwentaAurosClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value()) {
    auto mode = *call.get_mode();
    this->mode = mode;
    if (this->parent_ != nullptr) {
      if (mode == climate::CLIMATE_MODE_HEAT) {
        this->parent_->set_desired_season_mode(0);  // Winter
      } else if (mode == climate::CLIMATE_MODE_HEAT_COOL || mode == climate::CLIMATE_MODE_FAN_ONLY) {
        this->parent_->set_desired_season_mode(1);  // Summer
      } else if (mode == climate::CLIMATE_MODE_COOL) {
        this->parent_->set_desired_season_mode(2);  // Cooling
      }
    }
  }

  if (call.get_preset().has_value()) {
    auto preset = *call.get_preset();
    this->set_preset_(preset);
    if (this->parent_ != nullptr) {
      if (preset == climate::CLIMATE_PRESET_ECO) {
        this->parent_->set_desired_thermostat_mode_index(0);  // Schedule
      } else if (preset == climate::CLIMATE_PRESET_BOOST) {
        this->parent_->set_desired_thermostat_mode_index(1);  // Party
      } else if (preset == climate::CLIMATE_PRESET_HOME) {
        this->parent_->set_desired_thermostat_mode_index(2);  // Manual
      }
    }
  }

  if (call.get_fan_mode().has_value()) {
    auto fm = *call.get_fan_mode();
    this->set_fan_mode_(fm);
    if (this->parent_ != nullptr) {
      switch (fm) {
        case climate::CLIMATE_FAN_OFF:
          this->parent_->set_desired_fan_mode_index(0);
          break;
        case climate::CLIMATE_FAN_LOW:
          this->parent_->set_desired_fan_mode_index(1);
          break;
        case climate::CLIMATE_FAN_MEDIUM:
          this->parent_->set_desired_fan_mode_index(2);
          break;
        case climate::CLIMATE_FAN_HIGH:
          this->parent_->set_desired_fan_mode_index(3);
          break;
        case climate::CLIMATE_FAN_DIFFUSE:
          this->parent_->set_desired_fan_mode_index(4);
          break;
        case climate::CLIMATE_FAN_AUTO:
          this->parent_->set_desired_fan_mode_index(5);
          break;
        default:
          break;
      }
    }
  }

  if (call.get_target_temperature().has_value()) {
    float temp = *call.get_target_temperature();
    this->target_temperature = temp;
    this->set_preset_(climate::CLIMATE_PRESET_HOME);  // Manual
    if (this->parent_ != nullptr) {
      this->parent_->set_climate_target_temperature(temp);
    }
  }

  this->publish_state();
}

}  // namespace esphome::awenta_auros
#endif
