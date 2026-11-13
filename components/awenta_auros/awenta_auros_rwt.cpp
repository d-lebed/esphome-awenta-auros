#include "awenta_auros.h"

#include <cstdio>

namespace esphome::awenta_auros {

void AwentaAurosComponent::set_desired_register(uint16_t reg_id, int32_t desired_raw) {
  auto &reg = this->managed_registers_[reg_id];
  reg.desired_raw = desired_raw;

  if (reg.has_current && reg.current_raw == desired_raw) {
    reg.has_desired = false;
    ESP_LOGI(TAG_RWT, "Register %u already has value %d on panel, no write needed",
             reg_id, (int) desired_raw);
    return;
  }

  reg.has_desired = true;
  ESP_LOGW(TAG_RWT, "Scheduled register %u write: desired=%d (current=%s)",
           reg_id, (int) desired_raw,
           reg.has_current ? std::to_string(reg.current_raw).c_str() : "unknown");
}

void AwentaAurosComponent::update_register_current(uint16_t reg_id, int32_t current_raw) {
  auto &reg = this->managed_registers_[reg_id];
  reg.current_raw = current_raw;
  reg.has_current = true;

  if (reg.has_desired && reg.desired_raw == current_raw) {
    reg.has_desired = false;
    ESP_LOGW(TAG_RWT, "Register %u confirmed at desired value %d!", reg_id, (int) current_raw);
  }

  if (!reg.has_desired) {
    if (reg_id == REG_FAN_MODE) {
#ifdef USE_SELECT
      if (this->fan_mode_select_ != nullptr) {
        std::string mode_str = fan_mode_to_string(current_raw);
        if (!mode_str.empty()) {
          this->fan_mode_select_->publish_state(mode_str);
        }
      }
#endif
#ifdef USE_AWENTA_AUROS_CLIMATE
      if (this->climate_ != nullptr) {
        this->climate_->apply_bus_fan_mode(current_raw);
        this->climate_->publish_state();
      }
#endif
#ifdef USE_AWENTA_AUROS_FAN
      if (this->fan_ != nullptr) {
        this->fan_->apply_bus_fan_mode(current_raw);
        this->fan_->publish_state();
      }
#endif
    } else if (reg_id == REG_THERMOSTAT_MODE) {
#ifdef USE_SELECT
      if (this->thermostat_mode_select_ != nullptr) {
        std::string therm_str = thermostat_mode_to_string(current_raw);
        if (!therm_str.empty()) {
          this->thermostat_mode_select_->publish_state(therm_str);
        }
      }
#endif
#ifdef USE_AWENTA_AUROS_CLIMATE
      if (this->climate_ != nullptr) {
        this->climate_->apply_bus_preset(current_raw);
        this->climate_->publish_state();
      }
#endif
    } else if (reg_id == REG_AIRING) {
#ifdef USE_SWITCH
      if (this->airing_switch_ != nullptr) {
        this->airing_switch_->publish_state(current_raw != 0);
      }
#endif
    } else if (reg_id == REG_FIREPLACE) {
#ifdef USE_SWITCH
      if (this->fireplace_switch_ != nullptr) {
        this->fireplace_switch_->publish_state(current_raw != 0);
      }
#endif
    } else if (reg_id == REG_GHE) {
#ifdef USE_SWITCH
      if (this->ghe_switch_ != nullptr) {
        this->ghe_switch_->publish_state(current_raw != 0);
      }
#endif
    } else if (reg_id == REG_BYPASS_MODE) {
#ifdef USE_SELECT
      if (this->bypass_mode_select_ != nullptr) {
        std::string bypass_str = bypass_mode_to_string(current_raw);
        if (!bypass_str.empty()) {
          this->bypass_mode_select_->publish_state(bypass_str);
        }
      }
#endif
    }
#ifdef USE_NUMBER
    else if (reg_id == REG_COMFORT_TEMP && this->comfort_temperature_number_ != nullptr) {
      this->comfort_temperature_number_->publish_state(current_raw / 10.0f);
    } else if (reg_id == REG_ECO_WINTER_TEMP && this->eco_winter_temperature_number_ != nullptr) {
      this->eco_winter_temperature_number_->publish_state(current_raw / 10.0f);
    } else if (reg_id == REG_ECO_COOLING_TEMP && this->eco_cooling_temperature_number_ != nullptr) {
      this->eco_cooling_temperature_number_->publish_state(current_raw / 10.0f);
    } else if (reg_id == REG_MANUAL_TEMP && this->manual_temperature_number_ != nullptr) {
      this->manual_temperature_number_->publish_state(current_raw / 10.0f);
    } else if (reg_id == REG_AWAY_TEMP && this->away_temperature_number_ != nullptr) {
      this->away_temperature_number_->publish_state(current_raw / 10.0f);
    } else if (reg_id == REG_PREHEATER_MIN_TEMP &&
               this->preheater_min_temperature_number_ != nullptr) {
      this->preheater_min_temperature_number_->publish_state(current_raw * 0.5f);
    } else if (reg_id == REG_SUPPLY_FAN_SPEED_1 && this->supply_fan_speed_1_number_ != nullptr) {
      this->supply_fan_speed_1_number_->publish_state((float) current_raw);
    } else if (reg_id == REG_SUPPLY_FAN_SPEED_2 && this->supply_fan_speed_2_number_ != nullptr) {
      this->supply_fan_speed_2_number_->publish_state((float) current_raw);
    } else if (reg_id == REG_SUPPLY_FAN_SPEED_3 && this->supply_fan_speed_3_number_ != nullptr) {
      this->supply_fan_speed_3_number_->publish_state((float) current_raw);
    } else if (reg_id == REG_SUPPLY_FAN_AIRING_SPEED &&
               this->supply_fan_airing_speed_number_ != nullptr) {
      this->supply_fan_airing_speed_number_->publish_state((float) current_raw);
    } else if (reg_id == REG_EXHAUST_FAN_SPEED_1 && this->exhaust_fan_speed_1_number_ != nullptr) {
      this->exhaust_fan_speed_1_number_->publish_state((float) current_raw);
    } else if (reg_id == REG_EXHAUST_FAN_SPEED_2 && this->exhaust_fan_speed_2_number_ != nullptr) {
      this->exhaust_fan_speed_2_number_->publish_state((float) current_raw);
    } else if (reg_id == REG_EXHAUST_FAN_SPEED_3 && this->exhaust_fan_speed_3_number_ != nullptr) {
      this->exhaust_fan_speed_3_number_->publish_state((float) current_raw);
    } else if (reg_id == REG_EXHAUST_FAN_AIRING_SPEED &&
               this->exhaust_fan_airing_speed_number_ != nullptr) {
      this->exhaust_fan_airing_speed_number_->publish_state((float) current_raw);
    }
#endif
  }

  if (reg_id == REG_FAN_MODE || reg_id == REG_AIRING ||
      reg_id == REG_SUPPLY_FAN_SPEED_1 || reg_id == REG_SUPPLY_FAN_SPEED_2 ||
      reg_id == REG_SUPPLY_FAN_SPEED_3) {
    this->update_smooth_fan_speed_from_bus_();
  }
}

bool AwentaAurosComponent::has_pending_writes() const {
  for (const auto &kv : this->managed_registers_) {
    const auto &reg = kv.second;
    if (reg.has_desired && (!reg.has_current || reg.desired_raw != reg.current_raw)) {
      return true;
    }
  }
  return false;
}

void AwentaAurosComponent::set_desired_fan_mode(const std::string &option) {
  int32_t mode = fan_mode_from_string(option);
  if (mode < 0) {
    ESP_LOGW(TAG_RWT, "Unknown fan mode option: %s", option.c_str());
    return;
  }
  this->set_desired_fan_mode_index((size_t) mode);
}

void AwentaAurosComponent::set_desired_fan_mode_index(size_t index) {
  int32_t mode = (int32_t) index;
  this->set_desired_register(REG_FAN_MODE, mode);
  if (this->sfs_enabled_) {
    this->set_desired_register(REG_AIRING, 0);
#ifdef USE_SWITCH
    if (this->airing_switch_ != nullptr) {
      this->airing_switch_->publish_state(false);
    }
#endif
    this->reset_sfs_fan_speeds_to_defaults();
#ifdef USE_NUMBER
    if (this->smooth_fan_speed_number_ != nullptr) {
      if (mode == 0) {
        this->smooth_fan_speed_number_->publish_state(0.0f);
      } else if (mode == 1) {
        this->smooth_fan_speed_number_->publish_state((float) this->sfs_speed_1_default_);
      } else if (mode == 2) {
        this->smooth_fan_speed_number_->publish_state((float) this->sfs_speed_2_default_);
      } else if (mode == 3) {
        this->smooth_fan_speed_number_->publish_state((float) this->sfs_speed_3_default_);
      }
    }
#endif
  }
#ifdef USE_SELECT
  if (this->fan_mode_select_ != nullptr) {
    this->fan_mode_select_->publish_state(fan_mode_to_string(mode));
  }
#endif
#ifdef USE_AWENTA_AUROS_CLIMATE
  if (this->climate_ != nullptr) {
    this->climate_->apply_bus_fan_mode(mode);
    this->climate_->publish_state();
  }
#endif
#ifdef USE_AWENTA_AUROS_FAN
  if (this->fan_ != nullptr) {
    if (this->sfs_enabled_) {
      int32_t sp = 0;
      if (mode == 1)
        sp = this->sfs_speed_1_default_;
      else if (mode == 2)
        sp = this->sfs_speed_2_default_;
      else if (mode == 3)
        sp = this->sfs_speed_3_default_;
      else if (mode == 4 || mode == 5) {
        uint8_t g = this->last_bus_active_gear_;
        sp = (g == 1) ? this->sfs_speed_1_default_
                      : ((g == 2) ? this->sfs_speed_2_default_ : this->sfs_speed_3_default_);
      }
      this->fan_->apply_smooth_speed_state(sp, mode, false);
    } else {
      this->fan_->apply_bus_fan_mode(mode);
      this->fan_->publish_state();
    }
  }
#endif
}

void AwentaAurosComponent::set_desired_thermostat_mode(const std::string &option) {
  int32_t mode = thermostat_mode_from_string(option);
  if (mode < 0) {
    ESP_LOGW(TAG_RWT, "Unknown thermostat mode option: %s", option.c_str());
    return;
  }
  this->set_desired_thermostat_mode_index((size_t) mode);
}

void AwentaAurosComponent::set_desired_thermostat_mode_index(size_t index) {
  int32_t mode = (int32_t) index;
  this->set_desired_register(REG_THERMOSTAT_MODE, mode);
#ifdef USE_SELECT
  if (this->thermostat_mode_select_ != nullptr) {
    this->thermostat_mode_select_->publish_state(thermostat_mode_to_string(mode));
  }
#endif
#ifdef USE_AWENTA_AUROS_CLIMATE
  if (this->climate_ != nullptr) {
    this->climate_->apply_bus_preset(mode);
    if (mode == 2 && this->managed_registers_[REG_MANUAL_TEMP].has_current) {
      const auto &mreg = this->managed_registers_[REG_MANUAL_TEMP];
      this->climate_->target_temperature =
          (mreg.has_desired ? mreg.desired_raw : mreg.current_raw) / 10.0f;
    }
    this->climate_->publish_state();
  }
#endif
}

void AwentaAurosComponent::set_desired_airing(bool enabled) {
  this->set_desired_register(REG_AIRING, enabled ? 1 : 0);
}

void AwentaAurosComponent::set_desired_fireplace(bool enabled) {
  this->set_desired_register(REG_FIREPLACE, enabled ? 1 : 0);
}

void AwentaAurosComponent::set_desired_ghe(bool enabled) {
  this->set_desired_register(REG_GHE, enabled ? 1 : 0);
}

void AwentaAurosComponent::set_desired_bypass_mode(const std::string &option) {
  int32_t mode = bypass_mode_from_string(option);
  if (mode < 0) {
    ESP_LOGW(TAG_RWT, "Unknown bypass mode option: %s", option.c_str());
    return;
  }
  this->set_desired_bypass_mode_index((size_t) mode);
}

void AwentaAurosComponent::set_desired_bypass_mode_index(size_t index) {
  int32_t mode = (int32_t) index;
  this->set_desired_register(REG_BYPASS_MODE, mode);
#ifdef USE_SELECT
  if (this->bypass_mode_select_ != nullptr) {
    this->bypass_mode_select_->publish_state(bypass_mode_to_string(mode));
  }
#endif
}

void AwentaAurosComponent::set_desired_season_mode(int32_t season_mode) {
  if (season_mode < 0 || season_mode > 2)
    return;
  this->set_desired_register(REG_SEASON_MODE, season_mode);
}

void AwentaAurosComponent::set_desired_temperature(uint16_t reg_id, float temp_c) {
  int32_t raw = (int32_t) std::round(temp_c * 10.0f);
  this->set_desired_register(reg_id, raw);
#ifdef USE_AWENTA_AUROS_CLIMATE
  if (reg_id == REG_MANUAL_TEMP && this->climate_ != nullptr) {
    const auto &treg = this->managed_registers_[REG_THERMOSTAT_MODE];
    int32_t active_therm =
        treg.has_desired ? treg.desired_raw : (treg.has_current ? treg.current_raw : -1);
    if (active_therm == 2) {
      this->climate_->target_temperature = temp_c;
      this->climate_->publish_state();
    }
  }
#endif
}

void AwentaAurosComponent::set_desired_preheater_min_temperature(float temp_c) {
  int32_t raw = (int32_t) std::round(temp_c * 2.0f);
  if (raw < -20)
    raw = -20;
  if (raw > 20)
    raw = 20;
  this->set_desired_register(REG_PREHEATER_MIN_TEMP, raw);
}

void AwentaAurosComponent::set_desired_fan_speed_percent(uint16_t reg_id, float percent) {
  int32_t raw = (int32_t) std::round(percent);
  if (raw < 0)
    raw = 0;
  if (raw > 100)
    raw = 100;
  this->set_desired_register(reg_id, raw);
}

void AwentaAurosComponent::set_climate_target_temperature(float temp_c) {
  this->set_desired_temperature(REG_MANUAL_TEMP, temp_c);
  this->set_desired_register(REG_THERMOSTAT_MODE, 2);  // 2 = Manual
#ifdef USE_NUMBER
  if (this->manual_temperature_number_ != nullptr) {
    this->manual_temperature_number_->publish_state(temp_c);
  }
#endif
#ifdef USE_SELECT
  if (this->thermostat_mode_select_ != nullptr) {
    this->thermostat_mode_select_->publish_state("Manual");
  }
#endif
}

void AwentaAurosComponent::configure_smooth_fan_speed(uint8_t min_speed, uint8_t max_speed,
                                                      uint8_t speed_1_default,
                                                      uint8_t speed_2_default,
                                                      uint8_t speed_3_default) {
  this->sfs_enabled_ = true;
  this->sfs_min_speed_ = min_speed;
  this->sfs_max_speed_ = max_speed;
  this->sfs_speed_1_default_ = speed_1_default;
  this->sfs_speed_2_default_ = speed_2_default;
  this->sfs_speed_3_default_ = speed_3_default;
#ifdef USE_AWENTA_AUROS_FAN
  if (this->fan_ != nullptr) {
    this->fan_->enable_smooth_speed_mode(speed_1_default, speed_2_default, speed_3_default);
  }
#endif
}

void AwentaAurosComponent::publish_fan_speed_numbers_(int32_t s1, int32_t s2, int32_t s3) {
#ifdef USE_NUMBER
  if (this->supply_fan_speed_1_number_ != nullptr)
    this->supply_fan_speed_1_number_->publish_state((float) s1);
  if (this->supply_fan_speed_2_number_ != nullptr)
    this->supply_fan_speed_2_number_->publish_state((float) s2);
  if (this->supply_fan_speed_3_number_ != nullptr)
    this->supply_fan_speed_3_number_->publish_state((float) s3);
  if (this->exhaust_fan_speed_1_number_ != nullptr)
    this->exhaust_fan_speed_1_number_->publish_state((float) s1);
  if (this->exhaust_fan_speed_2_number_ != nullptr)
    this->exhaust_fan_speed_2_number_->publish_state((float) s2);
  if (this->exhaust_fan_speed_3_number_ != nullptr)
    this->exhaust_fan_speed_3_number_->publish_state((float) s3);
#endif
}

void AwentaAurosComponent::reset_sfs_fan_speeds_to_defaults() {
  if (!this->sfs_enabled_)
    return;

  int32_t s1 = this->sfs_speed_1_default_;
  int32_t s2 = this->sfs_speed_2_default_;
  int32_t s3 = this->sfs_speed_3_default_;

  this->set_desired_register(REG_SUPPLY_FAN_SPEED_1, s1);
  this->set_desired_register(REG_SUPPLY_FAN_SPEED_2, s2);
  this->set_desired_register(REG_SUPPLY_FAN_SPEED_3, s3);
  this->set_desired_register(REG_EXHAUST_FAN_SPEED_1, s1);
  this->set_desired_register(REG_EXHAUST_FAN_SPEED_2, s2);
  this->set_desired_register(REG_EXHAUST_FAN_SPEED_3, s3);

  this->publish_fan_speed_numbers_(s1, s2, s3);
}

void AwentaAurosComponent::set_desired_smooth_fan_speed(float percent) {
  int32_t target = (int32_t) std::round(percent);
  if (target < 0)
    target = 0;
  if (target > 100)
    target = 100;

  int32_t s1 = this->sfs_speed_1_default_;
  int32_t s2 = this->sfs_speed_2_default_;
  int32_t s3 = this->sfs_speed_3_default_;

  if (target == 0) {
    this->reset_sfs_fan_speeds_to_defaults();
    this->set_desired_register(REG_AIRING, 0);
    this->set_desired_register(REG_FAN_MODE, 0);
#ifdef USE_SWITCH
    if (this->airing_switch_ != nullptr)
      this->airing_switch_->publish_state(false);
#endif
#ifdef USE_SELECT
    if (this->fan_mode_select_ != nullptr)
      this->fan_mode_select_->publish_state(fan_mode_to_string(0));
#endif
#ifdef USE_AWENTA_AUROS_CLIMATE
    if (this->climate_ != nullptr) {
      this->climate_->apply_bus_fan_mode(0);
      this->climate_->publish_state();
    }
#endif
#ifdef USE_AWENTA_AUROS_FAN
    if (this->fan_ != nullptr) {
      this->fan_->apply_smooth_speed_state(0, 0, false);
    }
#endif
#ifdef USE_NUMBER
    if (this->smooth_fan_speed_number_ != nullptr)
      this->smooth_fan_speed_number_->publish_state(0.0f);
#endif
    return;
  }

  if (target >= 100) {
    this->reset_sfs_fan_speeds_to_defaults();
    this->set_desired_register(REG_AIRING, 1);
#ifdef USE_SWITCH
    if (this->airing_switch_ != nullptr)
      this->airing_switch_->publish_state(true);
#endif
#ifdef USE_AWENTA_AUROS_FAN
    if (this->fan_ != nullptr) {
      this->fan_->apply_smooth_speed_state(100, -1, true);
    }
#endif
#ifdef USE_NUMBER
    if (this->smooth_fan_speed_number_ != nullptr)
      this->smooth_fan_speed_number_->publish_state(100.0f);
#endif
    return;
  }

  if (target < (int32_t) this->sfs_min_speed_)
    target = this->sfs_min_speed_;
  if (target > (int32_t) this->sfs_max_speed_)
    target = this->sfs_max_speed_;

  int32_t span = (int32_t) this->sfs_max_speed_ - (int32_t) this->sfs_min_speed_ + 1;
  int32_t offset = target - (int32_t) this->sfs_min_speed_;
  int32_t gear = 1 + (offset * 3) / (span > 0 ? span : 1);
  if (gear < 1)
    gear = 1;
  if (gear > 3)
    gear = 3;

  if (gear == 1)
    s1 = target;
  else if (gear == 2)
    s2 = target;
  else
    s3 = target;

  this->set_desired_register(REG_SUPPLY_FAN_SPEED_1, s1);
  this->set_desired_register(REG_SUPPLY_FAN_SPEED_2, s2);
  this->set_desired_register(REG_SUPPLY_FAN_SPEED_3, s3);
  this->set_desired_register(REG_EXHAUST_FAN_SPEED_1, s1);
  this->set_desired_register(REG_EXHAUST_FAN_SPEED_2, s2);
  this->set_desired_register(REG_EXHAUST_FAN_SPEED_3, s3);
  this->set_desired_register(REG_AIRING, 0);
  this->set_desired_register(REG_FAN_MODE, gear);

  this->publish_fan_speed_numbers_(s1, s2, s3);
#ifdef USE_SWITCH
  if (this->airing_switch_ != nullptr)
    this->airing_switch_->publish_state(false);
#endif
#ifdef USE_SELECT
  if (this->fan_mode_select_ != nullptr)
    this->fan_mode_select_->publish_state(fan_mode_to_string(gear));
#endif
#ifdef USE_AWENTA_AUROS_CLIMATE
  if (this->climate_ != nullptr) {
    this->climate_->apply_bus_fan_mode(gear);
    this->climate_->publish_state();
  }
#endif
#ifdef USE_AWENTA_AUROS_FAN
  if (this->fan_ != nullptr) {
    this->fan_->apply_smooth_speed_state(target, gear, false);
  }
#endif
#ifdef USE_NUMBER
  if (this->smooth_fan_speed_number_ != nullptr)
    this->smooth_fan_speed_number_->publish_state((float) target);
#endif
}

void AwentaAurosComponent::update_smooth_fan_speed_from_bus_() {
  if (!this->sfs_enabled_)
    return;
  if (this->has_pending_writes())
    return;

  const auto &airing_reg = this->managed_registers_[REG_AIRING];
  if (airing_reg.has_current && airing_reg.current_raw != 0) {
#ifdef USE_NUMBER
    if (this->smooth_fan_speed_number_ != nullptr)
      this->smooth_fan_speed_number_->publish_state(100.0f);
#endif
#ifdef USE_AWENTA_AUROS_FAN
    if (this->fan_ != nullptr)
      this->fan_->apply_smooth_speed_state(100, -1, true);
#endif
    return;
  }

  const auto &fm_reg = this->managed_registers_[REG_FAN_MODE];
  int32_t fm = fm_reg.has_current ? fm_reg.current_raw : -1;
  int32_t gear = this->last_bus_active_gear_;
  if (fm_reg.has_current) {
    if (fm_reg.current_raw == 0) {
#ifdef USE_NUMBER
      if (this->smooth_fan_speed_number_ != nullptr)
        this->smooth_fan_speed_number_->publish_state(0.0f);
#endif
#ifdef USE_AWENTA_AUROS_FAN
      if (this->fan_ != nullptr)
        this->fan_->apply_smooth_speed_state(0, 0, false);
#endif
      return;
    }
    if (fm_reg.current_raw >= 1 && fm_reg.current_raw <= 3) {
      gear = fm_reg.current_raw;
    }
  }

  if (gear == 0) {
#ifdef USE_NUMBER
    if (this->smooth_fan_speed_number_ != nullptr)
      this->smooth_fan_speed_number_->publish_state(0.0f);
#endif
#ifdef USE_AWENTA_AUROS_FAN
    if (this->fan_ != nullptr)
      this->fan_->apply_smooth_speed_state(0, 0, false);
#endif
    return;
  }

  uint16_t speed_reg = (gear == 1) ? REG_SUPPLY_FAN_SPEED_1
                                   : ((gear == 2) ? REG_SUPPLY_FAN_SPEED_2 : REG_SUPPLY_FAN_SPEED_3);
  const auto &sp_reg = this->managed_registers_[speed_reg];
  if (sp_reg.has_current) {
#ifdef USE_NUMBER
    if (this->smooth_fan_speed_number_ != nullptr)
      this->smooth_fan_speed_number_->publish_state((float) sp_reg.current_raw);
#endif
#ifdef USE_AWENTA_AUROS_FAN
    if (this->fan_ != nullptr)
      this->fan_->apply_smooth_speed_state(sp_reg.current_raw, fm, false);
#endif
  }
}

void AwentaAurosComponent::handle_slots_(const std::vector<uint8_t> &rx_buffer) {
  for (int i = 0; i < 6; i++) {
    int index_id_h = 5 + (i * 4);
    int index_id_l = 6 + (i * 4);
    int index_val_h = 7 + (i * 4);
    int index_val_l = 8 + (i * 4);

    if (index_val_l < 29) {
      uint8_t h = rx_buffer[index_id_h];
      uint8_t l = rx_buffer[index_id_l];
      uint8_t val_h = rx_buffer[index_val_h];
      uint8_t val_l = rx_buffer[index_val_l];

      if (!(h == 0x00 && l == 0x00 && val_h == 0x00 && val_l == 0x00) &&
          !(h == 0x7E && l == 0x7E && val_h == 0x7E && val_l == 0x7E)) {
        uint16_t reg_index = decode_c14_index(h, l);
        int32_t raw_int = (int32_t) decode_c14_index(val_h, val_l) - 2000;
        float value_actual = raw_int / 10.0f;

        ESP_LOGI(TAG,
                 "  [B:%02d-%02d] | Slot [%5d] (H:%02X L:%02X) -> Value: %02X %02X (int:%d | %.1f)",
                 index_id_h, index_val_l, reg_index, h, l, val_h, val_l, (int) raw_int,
                 value_actual);
      }
    }
  }
}

void AwentaAurosComponent::send_c14_rwt_(uint8_t target_addr, uint8_t command,
                                         const std::array<uint16_t, 6> &regs,
                                         const std::array<uint16_t, 6> &d_raw_values,
                                         uint8_t sender_addr, const std::string &probe_label) {
  std::vector<uint8_t> frame(30, 0x00);
  frame[0] = target_addr + 128;
  frame[1] = command;
  frame[2] = 0x00;
  frame[3] = sender_addr;
  frame[4] = 0x00;

  for (int i = 0; i < 6; i++) {
    int base = 5 + (i * 4);
    encode_c14_index(regs[i], frame[base], frame[base + 1]);
    encode_c14_index(d_raw_values[i], frame[base + 2], frame[base + 3]);
  }

  frame[29] = 0x23;
  frame[2] = calculate_c14_checksum(frame);

  std::string hex_str;
  for (uint8_t b : frame) {
    char buf[4];
    snprintf(buf, sizeof(buf), "%02X ", b);
    hex_str += buf;
  }

  ESP_LOGW(TAG_RWT, ">>> TX [%s] cmd='%c' From:%d(0x%02X) -> To:%d(0x%02X) CRC:%02X | Raw: %s",
           probe_label.c_str(), command, sender_addr, sender_addr, target_addr,
           frame[0], frame[2], hex_str.c_str());

  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->digital_write(true);
  }
  delayMicroseconds(1000);
  this->write_array(frame);
  this->flush();
  delayMicroseconds(1000);
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->digital_write(false);
  }

  while (this->available()) {
    uint8_t dummy = this->read();
    (void) dummy;
  }
  this->rx_buffer_.clear();
}

void AwentaAurosComponent::execute_scheduled_write_() {
  std::array<uint16_t, 6> regs = {0, 0, 0, 0, 0, 0};
  std::array<uint16_t, 6> d_raw_values = {2000, 2000, 2000, 2000, 2000, 2000};
  int slot = 0;
  std::string label = "W:";

  for (const auto &kv : this->managed_registers_) {
    if (slot >= 6)
      break;
    uint16_t reg_id = kv.first;
    const auto &reg = kv.second;
    if (reg.has_desired && (!reg.has_current || reg.desired_raw != reg.current_raw)) {
      regs[slot] = reg_id;
      int32_t encoded_val = reg.desired_raw + 2000;
      if (encoded_val < 0)
        encoded_val = 0;
      d_raw_values[slot] = (uint16_t) encoded_val;

      char buf[32];
      snprintf(buf, sizeof(buf), "%u=%d ", reg_id, (int) reg.desired_raw);
      label += buf;
      slot++;
    }
  }

  if (slot == 0)
    return;

  this->send_c14_rwt_(ADDR_MASTER, 'W', regs, d_raw_values, ADDR_INEXT, label);
}

void AwentaAurosComponent::handle_rwt_response_(uint8_t command, uint8_t source_address,
                                                uint8_t target_address, const std::string &hex_str) {
  std::string slots_summary;

  for (int i = 0; i < 6; i++) {
    int base = 5 + (i * 4);
    uint16_t reg_id = decode_c14_index(this->rx_buffer_[base], this->rx_buffer_[base + 1]);
    int32_t raw_int =
        (int32_t) decode_c14_index(this->rx_buffer_[base + 2], this->rx_buffer_[base + 3]) - 2000;
    float val_f = raw_int / 10.0f;

    if (reg_id != 0 || raw_int != 0) {
      char sbuf[48];
      snprintf(sbuf, sizeof(sbuf), "[%u]=%d(%.1f) ", reg_id, (int) raw_int, val_f);
      slots_summary += sbuf;
    }

    if ((command == 'r' || command == 'w') && reg_id != 0 && raw_int != 9999) {
      this->update_register_current(reg_id, raw_int);
    }
  }

  char full_buf[256];
  snprintf(full_buf, sizeof(full_buf), "cmd='%c' %d->%d: %s| Raw: %s",
           command, source_address, target_address, slots_summary.c_str(), hex_str.c_str());

  ESP_LOGW(TAG_RWT, "<<< RWT RESPONSE RECEIVED! %s", full_buf);

  if (command == 'w' && this->has_pending_writes() && !this->tx_scheduled_) {
    uint32_t elapsed_ms = millis() - this->inext_window_start_ms_;
    if (elapsed_ms < 500) {
      this->tx_scheduled_ = true;
      this->scheduled_tx_time_ms_ = millis() + 40;
      ESP_LOGW(TAG_RWT,
               "More pending registers remain (%ums into iNext window), scheduling 2nd 'W' packet in 40ms",
               (unsigned) elapsed_ms);
    }
  }
}

}  // namespace esphome::awenta_auros
