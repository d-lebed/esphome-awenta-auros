#include "awenta_auros.h"

#include <cstdio>

namespace esphome::awenta_auros {

void AwentaAurosComponent::print_header_(uint8_t command, uint8_t source_address,
                                         uint8_t target_address, uint8_t checksum,
                                         const std::string &hex_str) {
  ESP_LOGI(TAG, "--- PACKET TYPE '%c' ---", command);
  ESP_LOGI(TAG, "From: %d (0x%02X) | To: %d (0x%02X) | CRC: %02X (OK)",
           source_address, source_address, target_address, target_address, checksum);
  ESP_LOGI(TAG, "Raw: %s", hex_str.c_str());
}

void AwentaAurosComponent::handle_master_status_(const std::vector<uint8_t> &rx_buffer) {
  uint8_t mode = rx_buffer[5];
  uint8_t day_of_week = rx_buffer[7];
  uint8_t time_h = rx_buffer[8];
  uint8_t time_l = rx_buffer[9];

  this->current_hour_ = time_h;
  this->current_minute_ = time_l;

  float room_temp = decode_c14_value(rx_buffer[12], rx_buffer[13]);
  float target_temp = decode_c14_value(rx_buffer[14], rx_buffer[15]);

  bool schedule_active = (mode & 0x10) != 0;
  this->is_away_active_ = (mode & 0x04) != 0;
#ifdef USE_BINARY_SENSOR
  if (this->schedule_mode_binary_sensor_ != nullptr) {
    this->schedule_mode_binary_sensor_->publish_state(schedule_active);
  }
  if (this->config_away_binary_sensor_ != nullptr) {
    this->config_away_binary_sensor_->publish_state(this->is_away_active_);
  }
#endif

#ifdef USE_TEXT_SENSOR
  if (this->day_of_week_text_sensor_ != nullptr) {
    const char *dow_names[] = {
        "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    std::string dow_str = (day_of_week <= 6) ? dow_names[day_of_week] : "Invalid";
    this->day_of_week_text_sensor_->publish_state(dow_str);
  }
#endif

  if (rx_buffer[25] == 0x01) {
    this->current_year_ = 2000 + rx_buffer[26];
  } else if (rx_buffer[25] == 0x02) {
    this->current_month_ = rx_buffer[26];
  } else if (rx_buffer[25] == 0x03) {
    this->current_day_ = rx_buffer[26];
  }

#ifdef USE_TEXT_SENSOR
  if (this->time_text_sensor_ != nullptr) {
    char time_buf[32];
    snprintf(time_buf, sizeof(time_buf), "%04d-%02d-%02d %02d:%02d",
             this->current_year_, this->current_month_, this->current_day_,
             this->current_hour_, this->current_minute_);
    this->time_text_sensor_->publish_state(std::string(time_buf));
  }
#endif

#ifdef USE_SENSOR
  if (this->target_temperature_sensor_ != nullptr)
    this->target_temperature_sensor_->publish_state(target_temp);
  if (this->room_temperature_sensor_ != nullptr)
    this->room_temperature_sensor_->publish_state(room_temp);
  if (this->room_humidity_sensor_ != nullptr)
    this->room_humidity_sensor_->publish_state(rx_buffer[23]);
#endif

  bool cooling_demand = (rx_buffer[24] & 0x32) == 0x00;
#ifdef USE_BINARY_SENSOR
  if (this->cooling_demand_binary_sensor_ != nullptr) {
    this->cooling_demand_binary_sensor_->publish_state(cooling_demand);
  }
#endif

  uint8_t therm_bits = rx_buffer[27] & 0x03;
#ifdef USE_TEXT_SENSOR
  if (this->thermostat_mode_text_sensor_ != nullptr) {
    std::string mode_str = thermostat_mode_to_string(therm_bits);
    if (mode_str.empty()) {
      mode_str = "Unexpected (" + std::to_string(therm_bits) + ")";
    }
    this->thermostat_mode_text_sensor_->publish_state(mode_str);
  }
#endif

  uint8_t season_bits = rx_buffer[27] & 0x18;
  int32_t season_idx = (season_bits == 0x00) ? 0 : ((season_bits == 0x08) ? 1 : 2);
  this->update_register_current(REG_SEASON_MODE, season_idx);

#ifdef USE_TEXT_SENSOR
  if (this->config_season_mode_text_sensor_ != nullptr) {
    if (season_bits == 0x00)
      this->config_season_mode_text_sensor_->publish_state("Winter");
    else if (season_bits == 0x08)
      this->config_season_mode_text_sensor_->publish_state("Summer");
    else if (season_bits == 0x10)
      this->config_season_mode_text_sensor_->publish_state("Cooling");
    else
      this->config_season_mode_text_sensor_->publish_state("Unknown");
  }
#endif

  bool airing_active = (rx_buffer[27] & 0x20) != 0;
  this->update_register_current(REG_AIRING, airing_active ? 1 : 0);

#ifdef USE_BINARY_SENSOR
  if (this->config_airing_binary_sensor_ != nullptr) {
    this->config_airing_binary_sensor_->publish_state(airing_active);
  }
#endif

  uint8_t raw_fs = rx_buffer[28] & 0x07;
  uint8_t active_gear = (raw_fs == 3) ? 1 : ((raw_fs == 5) ? 2 : ((raw_fs == 7) ? 3 : 0));
  this->last_bus_active_gear_ = active_gear;
  this->update_smooth_fan_speed_from_bus_();
#ifdef USE_FAN
  if (this->fan_ != nullptr && active_gear > 0) {
    this->fan_->apply_bus_active_gear(active_gear);
  }
#endif
#ifdef USE_TEXT_SENSOR
  if (this->active_fan_speed_text_sensor_ != nullptr) {
    std::string fs_str;
    if (raw_fs == 1)
      fs_str = "0";
    else if (raw_fs == 3)
      fs_str = "1";
    else if (raw_fs == 5)
      fs_str = "2";
    else if (raw_fs == 7)
      fs_str = "3";
    else
      fs_str = "Unexpected (" + std::to_string(raw_fs) + ")";
    this->active_fan_speed_text_sensor_->publish_state(fs_str);
  }
#endif

#ifdef USE_CLIMATE
  if (this->climate_ != nullptr) {
    this->climate_->current_temperature = room_temp;
    this->climate_->current_humidity = rx_buffer[23];

    if (!this->managed_registers_[REG_MANUAL_TEMP].has_desired &&
        !this->managed_registers_[REG_THERMOSTAT_MODE].has_desired) {
      this->climate_->target_temperature = target_temp;
    }

    if (!this->managed_registers_[REG_SEASON_MODE].has_desired) {
      if (season_idx == 0) {
        this->climate_->mode = climate::CLIMATE_MODE_HEAT;
      } else if (season_idx == 1) {
        this->climate_->mode = climate::CLIMATE_MODE_HEAT_COOL;
      } else {
        this->climate_->mode = climate::CLIMATE_MODE_COOL;
      }
    }

    if (raw_fs <= 1) {
      this->climate_->action = climate::CLIMATE_ACTION_IDLE;
    } else if (season_idx == 0) {
      this->climate_->action =
          (room_temp < target_temp) ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_FAN;
    } else if (season_idx == 1) {
      this->climate_->action = climate::CLIMATE_ACTION_FAN;
    } else {
      this->climate_->action =
          cooling_demand ? climate::CLIMATE_ACTION_COOLING : climate::CLIMATE_ACTION_FAN;
    }

    this->climate_->publish_state();
  }
#endif

  ESP_LOGD(TAG, "Unparsed panel status bytes: [6]=%02X [10]=%02X [11]=%02X",
           rx_buffer[6], rx_buffer[10], rx_buffer[11]);
}

void AwentaAurosComponent::handle_master_config_(const std::vector<uint8_t> &rx_buffer) {
  float config_intake_temp = decode_c14_value(rx_buffer[6], rx_buffer[7]);
  float config_comfort_temp = decode_c14_value(rx_buffer[8], rx_buffer[9]);
  float config_eco_winter_temp = decode_c14_value(rx_buffer[10], rx_buffer[11]);
  float config_eco_cooling_temp = decode_c14_value(rx_buffer[12], rx_buffer[13]);
  float config_manual_temp = decode_c14_value(rx_buffer[14], rx_buffer[15]);
  float config_away_temp = decode_c14_value(rx_buffer[16], rx_buffer[17]);

  this->update_register_current(REG_COMFORT_TEMP,
                                (int32_t) decode_c14_index(rx_buffer[8], rx_buffer[9]) - 2000);
  this->update_register_current(REG_ECO_WINTER_TEMP,
                                (int32_t) decode_c14_index(rx_buffer[10], rx_buffer[11]) - 2000);
  this->update_register_current(REG_ECO_COOLING_TEMP,
                                (int32_t) decode_c14_index(rx_buffer[12], rx_buffer[13]) - 2000);
  this->update_register_current(REG_MANUAL_TEMP,
                                (int32_t) decode_c14_index(rx_buffer[14], rx_buffer[15]) - 2000);
  this->update_register_current(REG_AWAY_TEMP,
                                (int32_t) decode_c14_index(rx_buffer[16], rx_buffer[17]) - 2000);

#ifdef USE_SENSOR
  if (this->config_intake_temperature_sensor_ != nullptr)
    this->config_intake_temperature_sensor_->publish_state(config_intake_temp);
  if (this->config_comfort_temperature_sensor_ != nullptr)
    this->config_comfort_temperature_sensor_->publish_state(config_comfort_temp);
  if (this->config_eco_winter_temperature_sensor_ != nullptr)
    this->config_eco_winter_temperature_sensor_->publish_state(config_eco_winter_temp);
  if (this->config_eco_cooling_temperature_sensor_ != nullptr)
    this->config_eco_cooling_temperature_sensor_->publish_state(config_eco_cooling_temp);
  if (this->config_manual_temperature_sensor_ != nullptr)
    this->config_manual_temperature_sensor_->publish_state(config_manual_temp);
  if (this->config_away_temperature_sensor_ != nullptr)
    this->config_away_temperature_sensor_->publish_state(config_away_temp);
#endif

  uint8_t raw_bypass = rx_buffer[25];
  uint8_t bypass = raw_bypass & 0x03;
  uint8_t ghe = raw_bypass & 0x04;
  this->update_register_current(REG_BYPASS_MODE, bypass);
  this->update_register_current(REG_GHE, (ghe != 0) ? 1 : 0);

#ifdef USE_TEXT_SENSOR
  if (this->config_bypass_mode_text_sensor_ != nullptr) {
    std::string bypass_str;
    if (bypass == 0)
      bypass_str = "Off";
    else if (bypass == 1)
      bypass_str = "Auto";
    else if (bypass == 2)
      bypass_str = "On";
    else
      bypass_str = "Unexpected (" + std::to_string(bypass) + ")";
    this->config_bypass_mode_text_sensor_->publish_state(bypass_str);
  }

  if (this->config_ghe_mode_text_sensor_ != nullptr) {
    std::string ghe_str;
    if (ghe == 0)
      ghe_str = "Off";
    else if (ghe == 4)
      ghe_str = "Auto";
    else
      ghe_str = "Unexpected (" + std::to_string(ghe) + ")";
    this->config_ghe_mode_text_sensor_->publish_state(ghe_str);
  }
#endif

  uint8_t raw_mode = rx_buffer[28];
  uint8_t fs = raw_mode % 10;
  uint8_t therm = (raw_mode / 10) % 10;

  this->update_register_current(REG_FAN_MODE, fs);
  this->update_register_current(REG_THERMOSTAT_MODE, therm);

#ifdef USE_TEXT_SENSOR
  if (this->config_fan_speed_text_sensor_ != nullptr) {
    std::string fs_str;
    if (fs == 0)
      fs_str = "0";
    else if (fs == 1)
      fs_str = "1";
    else if (fs == 2)
      fs_str = "2";
    else if (fs == 3)
      fs_str = "3";
    else if (fs == 4)
      fs_str = "Party";
    else if (fs == 5)
      fs_str = "Schedule";
    else
      fs_str = "Unexpected (" + std::to_string(fs) + ")";
    this->config_fan_speed_text_sensor_->publish_state(fs_str);
  }

  if (this->config_thermostat_mode_text_sensor_ != nullptr) {
    std::string therm_str;
    if (therm == 0)
      therm_str = "Schedule";
    else if (therm == 1)
      therm_str = "Party";
    else if (therm == 2)
      therm_str = "Manual";
    else
      therm_str = "Unexpected (" + std::to_string(therm) + ")";
    this->config_thermostat_mode_text_sensor_->publish_state(therm_str);
  }
#endif

  ESP_LOGD(TAG,
           "Unparsed master config bytes: [5]=%02X [18..24]=%02X %02X %02X %02X %02X %02X %02X [26]=%02X",
           rx_buffer[5], rx_buffer[18], rx_buffer[19], rx_buffer[20], rx_buffer[21],
           rx_buffer[22], rx_buffer[23], rx_buffer[24], rx_buffer[26]);
}

void AwentaAurosComponent::handle_aero_status_(const std::vector<uint8_t> &rx_buffer) {
  float intake_temp = decode_c14_value(rx_buffer[10], rx_buffer[11]);
  float supply_temp = decode_c14_value(rx_buffer[12], rx_buffer[13]);
  float preheater_temp = decode_c14_value(rx_buffer[14], rx_buffer[15]);
  float supply_target_temp = decode_c14_value(rx_buffer[18], rx_buffer[19]);

  this->last_preheater_temp_ = preheater_temp;
  this->last_supply_temp_ = supply_temp;
  this->has_primary_temps_ = true;
  this->publish_efficiency_sensors_();

  uint8_t target_fan_speed = rx_buffer[22];
  uint8_t intake_fan_speed = rx_buffer[25];
  uint8_t exhaust_fan_speed = rx_buffer[26];
  uint8_t bypass_pos = rx_buffer[28];

#ifdef USE_SENSOR
  if (this->intake_temperature_sensor_ != nullptr)
    this->intake_temperature_sensor_->publish_state(intake_temp);
  if (this->supply_temperature_sensor_ != nullptr)
    this->supply_temperature_sensor_->publish_state(supply_temp);
  if (this->preheater_temperature_sensor_ != nullptr)
    this->preheater_temperature_sensor_->publish_state(preheater_temp);
  if (this->target_fan_speed_sensor_ != nullptr)
    this->target_fan_speed_sensor_->publish_state(target_fan_speed);
  if (this->intake_fan_speed_sensor_ != nullptr)
    this->intake_fan_speed_sensor_->publish_state(intake_fan_speed);
  if (this->exhaust_fan_speed_sensor_ != nullptr)
    this->exhaust_fan_speed_sensor_->publish_state(exhaust_fan_speed);
  if (this->supply_target_temperature_sensor_ != nullptr)
    this->supply_target_temperature_sensor_->publish_state(supply_target_temp);
#endif

#ifdef USE_BINARY_SENSOR
  if (this->bypass_position_binary_sensor_ != nullptr) {
    this->bypass_position_binary_sensor_->publish_state((bypass_pos & 0x20) != 0);
  }
#endif

  ESP_LOGD(TAG,
           "Unparsed aero status bytes: [6..9]=%02X %02X %02X %02X [16..17]=%02X %02X [20..21]=%02X %02X [23..24]=%02X %02X [27]=%02X",
           rx_buffer[6], rx_buffer[7], rx_buffer[8], rx_buffer[9], rx_buffer[16], rx_buffer[17],
           rx_buffer[20], rx_buffer[21], rx_buffer[23], rx_buffer[24], rx_buffer[27]);
}

void AwentaAurosComponent::handle_aero_secondary_(const std::vector<uint8_t> &rx_buffer) {
  float exhaust_temp = decode_c14_value(rx_buffer[16], rx_buffer[17]);
  float extract_temp = decode_c14_value(rx_buffer[18], rx_buffer[19]);
  uint8_t supply_fan_speed = rx_buffer[14];
  uint8_t extract_fan_speed = rx_buffer[15];

  this->last_exhaust_temp_ = exhaust_temp;
  this->last_extract_temp_ = extract_temp;
  this->has_secondary_temps_ = true;
  this->publish_efficiency_sensors_();

#ifdef USE_SENSOR
  if (this->exhaust_temperature_sensor_ != nullptr)
    this->exhaust_temperature_sensor_->publish_state(exhaust_temp);
  if (this->extract_temperature_sensor_ != nullptr)
    this->extract_temperature_sensor_->publish_state(extract_temp);
  if (this->supply_fan_speed_sensor_ != nullptr)
    this->supply_fan_speed_sensor_->publish_state(supply_fan_speed);
  if (this->extract_fan_speed_sensor_ != nullptr)
    this->extract_fan_speed_sensor_->publish_state(extract_fan_speed);
#endif

  ESP_LOGD(TAG, "Unparsed aero secondary bytes: [5]=%02X [6..13]=%.1f %.1f %.1f %.1f [28]=%02X",
           rx_buffer[5], decode_c14_value(rx_buffer[6], rx_buffer[7]),
           decode_c14_value(rx_buffer[8], rx_buffer[9]),
           decode_c14_value(rx_buffer[10], rx_buffer[11]),
           decode_c14_value(rx_buffer[12], rx_buffer[13]), rx_buffer[28]);
}

void AwentaAurosComponent::publish_efficiency_sensors_() {
#ifdef USE_SENSOR
  if (!this->has_primary_temps_ || !this->has_secondary_temps_)
    return;
  if (this->supply_efficiency_sensor_ == nullptr && this->exhaust_efficiency_sensor_ == nullptr)
    return;

  float delta_max = this->last_extract_temp_ - this->last_preheater_temp_;
  if (std::fabs(delta_max) < 1.0f) {
    if (this->supply_efficiency_sensor_ != nullptr)
      this->supply_efficiency_sensor_->publish_state(NAN);
    if (this->exhaust_efficiency_sensor_ != nullptr)
      this->exhaust_efficiency_sensor_->publish_state(NAN);
    return;
  }

  if (this->supply_efficiency_sensor_ != nullptr) {
    float supply_eff =
        ((this->last_supply_temp_ - this->last_preheater_temp_) / delta_max) * 100.0f;
    if (supply_eff < 0.0f)
      supply_eff = 0.0f;
    if (supply_eff > 100.0f)
      supply_eff = 100.0f;
    this->supply_efficiency_sensor_->publish_state(supply_eff);
  }

  if (this->exhaust_efficiency_sensor_ != nullptr) {
    float exhaust_eff =
        ((this->last_extract_temp_ - this->last_exhaust_temp_) / delta_max) * 100.0f;
    if (exhaust_eff < 0.0f)
      exhaust_eff = 0.0f;
    if (exhaust_eff > 100.0f)
      exhaust_eff = 100.0f;
    this->exhaust_efficiency_sensor_->publish_state(exhaust_eff);
  }
#endif
}

void AwentaAurosComponent::parse_frame_() {
  if (this->rx_buffer_.size() != 30) {
    return;
  }

  uint8_t received_checksum = this->rx_buffer_[2];
  uint8_t calculated_checksum = calculate_c14_checksum(this->rx_buffer_);

  if (received_checksum != calculated_checksum) {
    ESP_LOGW(TAG, "Checksum error! Received: %02X, expected: %02X",
             received_checksum, calculated_checksum);
    return;
  }

  uint8_t command = this->rx_buffer_[1];
  uint8_t target_address =
      (this->rx_buffer_[0] >= 128) ? (this->rx_buffer_[0] - 128) : this->rx_buffer_[0];
  uint8_t source_address = this->rx_buffer_[3];
  std::string hex_str;
  for (uint8_t b : this->rx_buffer_) {
    char buf[4];
    snprintf(buf, sizeof(buf), "%02X ", b);
    hex_str += buf;
  }

  this->print_header_(command, source_address, target_address, received_checksum, hex_str);

  if (command == 'r' || command == 'w' || command == 't' ||
      ((target_address == ADDR_INEXT || target_address == ADDR_PC || target_address == ADDR_LAN) &&
       !(command == 'D' && this->rx_buffer_[4] == 0x7E))) {
    this->handle_rwt_response_(command, source_address, target_address, hex_str);
  }

  if (command == 'D' && source_address == ADDR_MASTER && target_address == ADDR_AERO) {
    bool fireplace_active = (this->rx_buffer_[28] & 0x40) != 0;
    this->update_register_current(REG_FIREPLACE, fireplace_active ? 1 : 0);
#ifdef USE_BINARY_SENSOR
    if (this->config_fireplace_mode_binary_sensor_ != nullptr) {
      this->config_fireplace_mode_binary_sensor_->publish_state(fireplace_active);
    }
#endif
    if (this->rx_buffer_[4] == 0x39) {
      int32_t exhaust_airing_speed = (int32_t) this->rx_buffer_[14];
      int32_t supply_airing_speed = (int32_t) this->rx_buffer_[15];
      int32_t exhaust_speed_1 = (int32_t) this->rx_buffer_[20];
      int32_t exhaust_speed_2 = (int32_t) this->rx_buffer_[21];
      int32_t exhaust_speed_3 = (int32_t) this->rx_buffer_[22];
      int32_t supply_speed_1 = (int32_t) this->rx_buffer_[23];
      int32_t supply_speed_2 = (int32_t) this->rx_buffer_[24];
      int32_t supply_speed_3 = (int32_t) this->rx_buffer_[25];

      this->update_register_current(REG_EXHAUST_FAN_AIRING_SPEED, exhaust_airing_speed);
      this->update_register_current(REG_SUPPLY_FAN_AIRING_SPEED, supply_airing_speed);
      this->update_register_current(REG_EXHAUST_FAN_SPEED_1, exhaust_speed_1);
      this->update_register_current(REG_EXHAUST_FAN_SPEED_2, exhaust_speed_2);
      this->update_register_current(REG_EXHAUST_FAN_SPEED_3, exhaust_speed_3);
      this->update_register_current(REG_SUPPLY_FAN_SPEED_1, supply_speed_1);
      this->update_register_current(REG_SUPPLY_FAN_SPEED_2, supply_speed_2);
      this->update_register_current(REG_SUPPLY_FAN_SPEED_3, supply_speed_3);

#ifdef USE_SENSOR
      if (this->config_exhaust_fan_airing_speed_sensor_ != nullptr)
        this->config_exhaust_fan_airing_speed_sensor_->publish_state((float) exhaust_airing_speed);
      if (this->config_supply_fan_airing_speed_sensor_ != nullptr)
        this->config_supply_fan_airing_speed_sensor_->publish_state((float) supply_airing_speed);
      if (this->config_exhaust_fan_speed_1_sensor_ != nullptr)
        this->config_exhaust_fan_speed_1_sensor_->publish_state((float) exhaust_speed_1);
      if (this->config_exhaust_fan_speed_2_sensor_ != nullptr)
        this->config_exhaust_fan_speed_2_sensor_->publish_state((float) exhaust_speed_2);
      if (this->config_exhaust_fan_speed_3_sensor_ != nullptr)
        this->config_exhaust_fan_speed_3_sensor_->publish_state((float) exhaust_speed_3);
      if (this->config_supply_fan_speed_1_sensor_ != nullptr)
        this->config_supply_fan_speed_1_sensor_->publish_state((float) supply_speed_1);
      if (this->config_supply_fan_speed_2_sensor_ != nullptr)
        this->config_supply_fan_speed_2_sensor_->publish_state((float) supply_speed_2);
      if (this->config_supply_fan_speed_3_sensor_ != nullptr)
        this->config_supply_fan_speed_3_sensor_->publish_state((float) supply_speed_3);
#endif
      ESP_LOGD(TAG,
               "Unparsed E3 44 (0x39) bytes: [5..13]=%02X %02X %02X %02X %02X %02X %02X %02X %02X [16..19]=%02X %02X %02X %02X [26]=%02X",
               this->rx_buffer_[5], this->rx_buffer_[6], this->rx_buffer_[7], this->rx_buffer_[8],
               this->rx_buffer_[9], this->rx_buffer_[10], this->rx_buffer_[11], this->rx_buffer_[12],
               this->rx_buffer_[13], this->rx_buffer_[16], this->rx_buffer_[17], this->rx_buffer_[18],
               this->rx_buffer_[19], this->rx_buffer_[26]);
    } else if (this->rx_buffer_[4] == 0x3A) {
      int32_t preheater_min_raw = (int32_t) this->rx_buffer_[12] - 20;
      float preheater_min_temp = preheater_min_raw * 0.5f;
      this->update_register_current(REG_PREHEATER_MIN_TEMP, preheater_min_raw);
#ifdef USE_SENSOR
      if (this->config_preheater_min_temperature_sensor_ != nullptr) {
        this->config_preheater_min_temperature_sensor_->publish_state(preheater_min_temp);
      }
#endif
      ESP_LOGD(TAG,
               "Unparsed E3 44 (0x3A) bytes: [5]=%02X [11]=%02X [13..27]=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
               this->rx_buffer_[5], this->rx_buffer_[11], this->rx_buffer_[13], this->rx_buffer_[14],
               this->rx_buffer_[15], this->rx_buffer_[16], this->rx_buffer_[17], this->rx_buffer_[18],
               this->rx_buffer_[19], this->rx_buffer_[20], this->rx_buffer_[21], this->rx_buffer_[22],
               this->rx_buffer_[23], this->rx_buffer_[24], this->rx_buffer_[25], this->rx_buffer_[26],
               this->rx_buffer_[27]);
    } else if (this->rx_buffer_[4] == 0x3B) {
      ESP_LOGD(TAG,
               "Unparsed E3 44 (0x3B) bytes: [5..27]=%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
               this->rx_buffer_[5], this->rx_buffer_[6], this->rx_buffer_[7], this->rx_buffer_[8],
               this->rx_buffer_[9], this->rx_buffer_[10], this->rx_buffer_[11], this->rx_buffer_[12],
               this->rx_buffer_[13], this->rx_buffer_[14], this->rx_buffer_[15], this->rx_buffer_[16],
               this->rx_buffer_[17], this->rx_buffer_[18], this->rx_buffer_[19], this->rx_buffer_[20],
               this->rx_buffer_[21], this->rx_buffer_[22], this->rx_buffer_[23], this->rx_buffer_[24],
               this->rx_buffer_[25], this->rx_buffer_[26], this->rx_buffer_[27]);
    }
  }

  if (command == 'D' && source_address == ADDR_MASTER && target_address == ADDR_INEXT &&
      this->has_pending_writes()) {
    this->inext_window_start_ms_ = millis();
    this->tx_scheduled_ = true;
    this->scheduled_tx_time_ms_ = this->inext_window_start_ms_ + 40;
    ESP_LOGW(TAG_RWT,
             "Detected iNext slot (F0 44 from Master 41 -> 112), scheduling pending register write in 40ms");
  }

  if (command == '!') {
    if (source_address == ADDR_MASTER && target_address == ADDR_BROADCAST_PRIMARY) {
      this->handle_master_status_(this->rx_buffer_);
      return;
    } else if (source_address == ADDR_AERO && target_address == ADDR_BROADCAST_PRIMARY) {
      this->handle_aero_status_(this->rx_buffer_);
      return;
    } else if (source_address == ADDR_AERO && target_address == ADDR_BROADCAST_SECONDARY) {
      this->handle_aero_secondary_(this->rx_buffer_);
      return;
    } else if (source_address == ADDR_MASTER && target_address == ADDR_BROADCAST_SECONDARY) {
      this->handle_master_config_(this->rx_buffer_);
      return;
    }
  }

  if (command == 'R' || command == 'r' || command == 'W' || command == 'w' ||
      command == 'T' || command == 't') {
    this->handle_slots_(this->rx_buffer_);
  }
}

}  // namespace esphome::awenta_auros
