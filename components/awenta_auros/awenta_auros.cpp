#include "awenta_auros.h"
#include <cstdio>

namespace esphome::awenta_auros
{

  static const char *const TAG = "awenta_auros";
  static const char *const TAG_RWT = "awenta_auros_rwt";

#ifdef USE_CLIMATE
  AwentaAurosClimate::AwentaAurosClimate() = default;

  void AwentaAurosClimate::apply_bus_fan_mode(int32_t fs)
  {
    switch (fs)
    {
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
      this->set_fan_mode_(climate::CLIMATE_FAN_DIFFUSE); // Party
      break;
    case 5:
      this->set_fan_mode_(climate::CLIMATE_FAN_AUTO); // Schedule
      break;
    }
  }

  void AwentaAurosClimate::apply_bus_preset(int32_t therm)
  {
    switch (therm)
    {
    case 0:
      this->set_preset_(climate::CLIMATE_PRESET_ECO); // Schedule
      break;
    case 1:
      this->set_preset_(climate::CLIMATE_PRESET_BOOST); // Party
      break;
    case 2:
      this->set_preset_(climate::CLIMATE_PRESET_HOME); // Manual
      break;
    }
  }

  climate::ClimateTraits AwentaAurosClimate::traits()
  {
    auto traits = climate::ClimateTraits();
    traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE |
                             climate::CLIMATE_SUPPORTS_CURRENT_HUMIDITY |
                             climate::CLIMATE_SUPPORTS_ACTION);
    traits.set_supported_modes({
        climate::CLIMATE_MODE_HEAT,
        climate::CLIMATE_MODE_COOL,
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

  void AwentaAurosClimate::control(const climate::ClimateCall &call)
  {
    if (call.get_mode().has_value())
    {
      auto mode = *call.get_mode();
      this->mode = mode;
      if (this->parent_ != nullptr)
      {
        if (mode == climate::CLIMATE_MODE_HEAT)
        {
          this->parent_->set_desired_season_mode(0); // Winter
        }
        else if (mode == climate::CLIMATE_MODE_FAN_ONLY)
        {
          this->parent_->set_desired_season_mode(1); // Summer
        }
        else if (mode == climate::CLIMATE_MODE_COOL)
        {
          this->parent_->set_desired_season_mode(2); // Cooling
        }
      }
    }

    if (call.get_preset().has_value())
    {
      auto preset = *call.get_preset();
      this->set_preset_(preset);
      if (this->parent_ != nullptr)
      {
        if (preset == climate::CLIMATE_PRESET_ECO)
        {
          this->parent_->set_desired_thermostat_mode_index(0); // Schedule
        }
        else if (preset == climate::CLIMATE_PRESET_BOOST)
        {
          this->parent_->set_desired_thermostat_mode_index(1); // Party
        }
        else if (preset == climate::CLIMATE_PRESET_HOME)
        {
          this->parent_->set_desired_thermostat_mode_index(2); // Manual
        }
      }
    }

    if (call.get_fan_mode().has_value())
    {
      auto fm = *call.get_fan_mode();
      this->set_fan_mode_(fm);
      if (this->parent_ != nullptr)
      {
        switch (fm)
        {
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

    if (call.get_target_temperature().has_value())
    {
      float temp = *call.get_target_temperature();
      this->target_temperature = temp;
      this->set_preset_(climate::CLIMATE_PRESET_HOME); // Manual
      if (this->parent_ != nullptr)
      {
        this->parent_->set_climate_target_temperature(temp);
      }
    }

    this->publish_state();
  }
#endif

  void AwentaAurosComponent::setup()
  {
    this->rx_buffer_.reserve(128);
    if (this->flow_control_pin_ != nullptr)
    {
      this->flow_control_pin_->setup();
      this->flow_control_pin_->digital_write(false);
    }
  }

  void AwentaAurosComponent::dump_config()
  {
    ESP_LOGCONFIG(TAG, "Awenta Auros C14 Component:");
    LOG_PIN("  Flow Control Pin: ", this->flow_control_pin_);
  }

  void AwentaAurosComponent::loop()
  {
    if (this->tx_scheduled_ && (int32_t)(millis() - this->scheduled_tx_time_ms_) >= 0)
    {
      this->tx_scheduled_ = false;
      this->execute_scheduled_write_();
    }

    while (this->available())
    {
      uint8_t byte = this->read();

      if (byte == 0xFF)
      {
        continue;
      }

      if (byte >= 0x80)
      {
        if (!this->rx_buffer_.empty())
        {
          ESP_LOGD(TAG, "Resync on new frame header 0x%02X (discarded %d incomplete bytes)",
                   byte, (int)this->rx_buffer_.size());
          this->rx_buffer_.clear();
        }
        this->rx_buffer_.push_back(byte);
        continue;
      }

      if (this->rx_buffer_.empty())
      {
        continue;
      }

      this->rx_buffer_.push_back(byte);

      if (this->rx_buffer_.size() == 30)
      {
        if (this->rx_buffer_[29] == 0x23)
        {
          this->parse_frame_();
        }
        else
        {
          ESP_LOGW(TAG, "30-byte buffer did not end with 0x23 (got 0x%02X), discarding.",
                   this->rx_buffer_[29]);
        }
        this->rx_buffer_.clear();
      }
    }
  }

  float AwentaAurosComponent::decode_value_(uint8_t h, uint8_t l)
  {
    int32_t value_raw = ((h & 0x7F) * 128) + (l & 0x7F) - 2000;
    return value_raw / 10.0f;
  }

  void AwentaAurosComponent::encode_value_(float value, uint8_t &h, uint8_t &l)
  {
    int32_t raw_value = (int32_t)std::round(value * 10.0f) + 2000;
    if (raw_value < 0)
      raw_value = 0;
    h = (raw_value / 128) & 0x7F;
    l = (raw_value % 128) & 0x7F;
  }

  uint16_t AwentaAurosComponent::decode_index_(uint8_t h, uint8_t l)
  {
    return ((h & 0x7F) * 128) + (l & 0x7F);
  }

  void AwentaAurosComponent::encode_index_(uint16_t index, uint8_t &h, uint8_t &l)
  {
    h = (index / 128) & 0x7F;
    l = (index % 128) & 0x7F;
  }

  uint8_t AwentaAurosComponent::calculate_c14_checksum_(const std::vector<uint8_t> &frame)
  {
    uint32_t arithmetic_sum = 0;
    for (size_t i = 0; i < frame.size() && i < 30; i++)
    {
      if (i == 2)
        continue;
      arithmetic_sum += frame[i];
    }
    return arithmetic_sum & 0x7F;
  }

  std::string AwentaAurosComponent::fan_mode_to_string(int32_t mode)
  {
    switch (mode)
    {
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

  int32_t AwentaAurosComponent::fan_mode_from_string(const std::string &option)
  {
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

  std::string AwentaAurosComponent::thermostat_mode_to_string(int32_t mode)
  {
    switch (mode)
    {
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

  int32_t AwentaAurosComponent::thermostat_mode_from_string(const std::string &option)
  {
    if (option == "Schedule")
      return 0;
    if (option == "Party" || option == "Holiday")
      return 1;
    if (option == "Manual")
      return 2;
    return -1;
  }

  std::string AwentaAurosComponent::bypass_mode_to_string(int32_t mode)
  {
    switch (mode)
    {
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

  int32_t AwentaAurosComponent::bypass_mode_from_string(const std::string &option)
  {
    if (option == "Off")
      return 0;
    if (option == "Auto")
      return 1;
    if (option == "On")
      return 2;
    return -1;
  }

  void AwentaAurosComponent::set_desired_register(uint16_t reg_id, int32_t desired_raw)
  {
    auto &reg = this->managed_registers_[reg_id];
    reg.desired_raw = desired_raw;

    if (reg.has_current && reg.current_raw == desired_raw)
    {
      reg.has_desired = false;
      ESP_LOGI(TAG_RWT, "Register %u already has value %d on panel, no write needed",
               reg_id, (int)desired_raw);
      return;
    }

    reg.has_desired = true;
    ESP_LOGW(TAG_RWT, "Scheduled register %u write: desired=%d (current=%s)",
             reg_id, (int)desired_raw,
             reg.has_current ? std::to_string(reg.current_raw).c_str() : "unknown");
  }

  void AwentaAurosComponent::update_register_current(uint16_t reg_id, int32_t current_raw)
  {
    auto &reg = this->managed_registers_[reg_id];
    reg.current_raw = current_raw;
    reg.has_current = true;

    if (reg.has_desired && reg.desired_raw == current_raw)
    {
      reg.has_desired = false;
      ESP_LOGW(TAG_RWT, "Register %u confirmed at desired value %d!", reg_id, (int)current_raw);
    }

    if (!reg.has_desired)
    {
      if (reg_id == REG_FAN_MODE)
      {
#ifdef USE_SELECT
        if (this->fan_mode_select_ != nullptr)
        {
          std::string mode_str = fan_mode_to_string(current_raw);
          if (!mode_str.empty())
          {
            this->fan_mode_select_->publish_state(mode_str);
          }
        }
#endif
#ifdef USE_CLIMATE
        if (this->climate_ != nullptr)
        {
          this->climate_->apply_bus_fan_mode(current_raw);
          this->climate_->publish_state();
        }
#endif
      }
      else if (reg_id == REG_THERMOSTAT_MODE)
      {
#ifdef USE_SELECT
        if (this->thermostat_mode_select_ != nullptr)
        {
          std::string therm_str = thermostat_mode_to_string(current_raw);
          if (!therm_str.empty())
          {
            this->thermostat_mode_select_->publish_state(therm_str);
          }
        }
#endif
#ifdef USE_CLIMATE
        if (this->climate_ != nullptr)
        {
          this->climate_->apply_bus_preset(current_raw);
          this->climate_->publish_state();
        }
#endif
      }
      else if (reg_id == REG_AIRING)
      {
#ifdef USE_SWITCH
        if (this->airing_switch_ != nullptr)
        {
          this->airing_switch_->publish_state(current_raw != 0);
        }
#endif
      }
      else if (reg_id == REG_FIREPLACE)
      {
#ifdef USE_SWITCH
        if (this->fireplace_switch_ != nullptr)
        {
          this->fireplace_switch_->publish_state(current_raw != 0);
        }
#endif
      }
      else if (reg_id == REG_GHE)
      {
#ifdef USE_SWITCH
        if (this->ghe_switch_ != nullptr)
        {
          this->ghe_switch_->publish_state(current_raw != 0);
        }
#endif
      }
      else if (reg_id == REG_BYPASS_MODE)
      {
#ifdef USE_SELECT
        if (this->bypass_mode_select_ != nullptr)
        {
          std::string bypass_str = bypass_mode_to_string(current_raw);
          if (!bypass_str.empty())
          {
            this->bypass_mode_select_->publish_state(bypass_str);
          }
        }
#endif
      }
#ifdef USE_NUMBER
      else if (reg_id == REG_COMFORT_TEMP && this->comfort_temperature_number_ != nullptr)
      {
        this->comfort_temperature_number_->publish_state(current_raw / 10.0f);
      }
      else if (reg_id == REG_ECO_WINTER_TEMP && this->eco_winter_temperature_number_ != nullptr)
      {
        this->eco_winter_temperature_number_->publish_state(current_raw / 10.0f);
      }
      else if (reg_id == REG_ECO_COOLING_TEMP && this->eco_cooling_temperature_number_ != nullptr)
      {
        this->eco_cooling_temperature_number_->publish_state(current_raw / 10.0f);
      }
      else if (reg_id == REG_MANUAL_TEMP && this->manual_temperature_number_ != nullptr)
      {
        this->manual_temperature_number_->publish_state(current_raw / 10.0f);
      }
      else if (reg_id == REG_AWAY_TEMP && this->away_temperature_number_ != nullptr)
      {
        this->away_temperature_number_->publish_state(current_raw / 10.0f);
      }
#endif
    }
  }

  bool AwentaAurosComponent::has_pending_writes() const
  {
    for (const auto &kv : this->managed_registers_)
    {
      const auto &reg = kv.second;
      if (reg.has_desired && (!reg.has_current || reg.desired_raw != reg.current_raw))
      {
        return true;
      }
    }
    return false;
  }

  void AwentaAurosComponent::set_desired_fan_mode(const std::string &option)
  {
    int32_t mode = fan_mode_from_string(option);
    if (mode < 0)
    {
      ESP_LOGW(TAG_RWT, "Unknown fan mode option: %s", option.c_str());
      return;
    }
    this->set_desired_fan_mode_index((size_t)mode);
  }

  void AwentaAurosComponent::set_desired_fan_mode_index(size_t index)
  {
    int32_t mode = (int32_t)index;
    this->set_desired_register(REG_FAN_MODE, mode);
#ifdef USE_SELECT
    if (this->fan_mode_select_ != nullptr)
    {
      this->fan_mode_select_->publish_state(fan_mode_to_string(mode));
    }
#endif
#ifdef USE_CLIMATE
    if (this->climate_ != nullptr)
    {
      this->climate_->apply_bus_fan_mode(mode);
      this->climate_->publish_state();
    }
#endif
  }

  void AwentaAurosComponent::set_desired_thermostat_mode(const std::string &option)
  {
    int32_t mode = thermostat_mode_from_string(option);
    if (mode < 0)
    {
      ESP_LOGW(TAG_RWT, "Unknown thermostat mode option: %s", option.c_str());
      return;
    }
    this->set_desired_thermostat_mode_index((size_t)mode);
  }

  void AwentaAurosComponent::set_desired_thermostat_mode_index(size_t index)
  {
    int32_t mode = (int32_t)index;
    this->set_desired_register(REG_THERMOSTAT_MODE, mode);
#ifdef USE_SELECT
    if (this->thermostat_mode_select_ != nullptr)
    {
      this->thermostat_mode_select_->publish_state(thermostat_mode_to_string(mode));
    }
#endif
#ifdef USE_CLIMATE
    if (this->climate_ != nullptr)
    {
      this->climate_->apply_bus_preset(mode);
      if (mode == 2 && this->managed_registers_[REG_MANUAL_TEMP].has_current)
      {
        const auto &mreg = this->managed_registers_[REG_MANUAL_TEMP];
        this->climate_->target_temperature =
            (mreg.has_desired ? mreg.desired_raw : mreg.current_raw) / 10.0f;
      }
      this->climate_->publish_state();
    }
#endif
  }

  void AwentaAurosComponent::set_desired_airing(bool enabled)
  {
    this->set_desired_register(REG_AIRING, enabled ? 1 : 0);
  }

  void AwentaAurosComponent::set_desired_fireplace(bool enabled)
  {
    this->set_desired_register(REG_FIREPLACE, enabled ? 1 : 0);
  }

  void AwentaAurosComponent::set_desired_ghe(bool enabled)
  {
    this->set_desired_register(REG_GHE, enabled ? 1 : 0);
  }

  void AwentaAurosComponent::set_desired_bypass_mode(const std::string &option)
  {
    int32_t mode = bypass_mode_from_string(option);
    if (mode < 0)
    {
      ESP_LOGW(TAG_RWT, "Unknown bypass mode option: %s", option.c_str());
      return;
    }
    this->set_desired_bypass_mode_index((size_t)mode);
  }

  void AwentaAurosComponent::set_desired_bypass_mode_index(size_t index)
  {
    int32_t mode = (int32_t)index;
    this->set_desired_register(REG_BYPASS_MODE, mode);
#ifdef USE_SELECT
    if (this->bypass_mode_select_ != nullptr)
    {
      this->bypass_mode_select_->publish_state(bypass_mode_to_string(mode));
    }
#endif
  }

  void AwentaAurosComponent::set_desired_season_mode(int32_t season_mode)
  {
    if (season_mode < 0 || season_mode > 2)
      return;
    this->set_desired_register(REG_SEASON_MODE, season_mode);
  }

  void AwentaAurosComponent::set_desired_temperature(uint16_t reg_id, float temp_c)
  {
    int32_t raw = (int32_t)std::round(temp_c * 10.0f);
    this->set_desired_register(reg_id, raw);
#ifdef USE_CLIMATE
    if (reg_id == REG_MANUAL_TEMP && this->climate_ != nullptr)
    {
      const auto &treg = this->managed_registers_[REG_THERMOSTAT_MODE];
      int32_t active_therm =
          treg.has_desired ? treg.desired_raw : (treg.has_current ? treg.current_raw : -1);
      if (active_therm == 2)
      {
        this->climate_->target_temperature = temp_c;
        this->climate_->publish_state();
      }
    }
#endif
  }

  void AwentaAurosComponent::set_climate_target_temperature(float temp_c)
  {
    this->set_desired_temperature(REG_MANUAL_TEMP, temp_c);
    this->set_desired_register(REG_THERMOSTAT_MODE, 2); // 2 = Manual
#ifdef USE_NUMBER
    if (this->manual_temperature_number_ != nullptr)
    {
      this->manual_temperature_number_->publish_state(temp_c);
    }
#endif
#ifdef USE_SELECT
    if (this->thermostat_mode_select_ != nullptr)
    {
      this->thermostat_mode_select_->publish_state("Manual");
    }
#endif
  }

  void AwentaAurosComponent::print_header_(uint8_t command, uint8_t source_address,
                                           uint8_t target_address, uint8_t checksum,
                                           const std::string &hex_str)
  {
    ESP_LOGI(TAG, "--- PACKET TYPE '%c' ---", command);
    ESP_LOGI(TAG, "From: %d (0x%02X) | To: %d (0x%02X) | CRC: %02X (OK)",
             source_address, source_address, target_address, target_address, checksum);
    ESP_LOGI(TAG, "Raw: %s", hex_str.c_str());
  }

  void AwentaAurosComponent::handle_master_status_(const std::vector<uint8_t> &rx_buffer)
  {
    uint8_t mode = rx_buffer[5];
    uint8_t day_of_week = rx_buffer[7];
    uint8_t time_h = rx_buffer[8];
    uint8_t time_l = rx_buffer[9];

    this->current_hour_ = time_h;
    this->current_minute_ = time_l;

    float room_temp = decode_value_(rx_buffer[12], rx_buffer[13]);
    float target_temp = decode_value_(rx_buffer[14], rx_buffer[15]);

#ifdef USE_TEXT_SENSOR
    if (this->thermostat_mode_text_sensor_ != nullptr)
    {
      uint8_t base_mode = mode & 0x50;
      std::string mode_str;
      switch (base_mode)
      {
      case 0x40:
        mode_str = "Manual";
        break;
      case 0x50:
        mode_str = "Schedule";
        break;
      default:
        mode_str = "Unexpected (" + std::to_string(base_mode) + ")";
        break;
      }
      this->thermostat_mode_text_sensor_->publish_state(mode_str);
    }
#endif

    this->is_away_active_ = (mode & 0x04) != 0;
#ifdef USE_BINARY_SENSOR
    if (this->away_binary_sensor_ != nullptr)
    {
      this->away_binary_sensor_->publish_state(this->is_away_active_);
    }
#endif

#ifdef USE_TEXT_SENSOR
    if (this->day_of_week_text_sensor_ != nullptr)
    {
      const char *dow_names[] = {
          "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
      std::string dow_str = (day_of_week <= 6) ? dow_names[day_of_week] : "Invalid";
      this->day_of_week_text_sensor_->publish_state(dow_str);
    }
#endif

    if (rx_buffer[25] == 0x01)
    {
      this->current_year_ = 2000 + rx_buffer[26];
    }
    else if (rx_buffer[25] == 0x02)
    {
      this->current_month_ = rx_buffer[26];
    }
    else if (rx_buffer[25] == 0x03)
    {
      this->current_day_ = rx_buffer[26];
    }

#ifdef USE_TEXT_SENSOR
    if (this->time_text_sensor_ != nullptr)
    {
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
    if (this->cooling_demand_binary_sensor_ != nullptr)
    {
      this->cooling_demand_binary_sensor_->publish_state(cooling_demand);
    }
#endif

    uint8_t season_bits = rx_buffer[27] & 0x18;
    int32_t season_idx = (season_bits == 0x00) ? 0 : ((season_bits == 0x08) ? 1 : 2);
    this->update_register_current(REG_SEASON_MODE, season_idx);

#ifdef USE_TEXT_SENSOR
    if (this->season_mode_text_sensor_ != nullptr)
    {
      if (season_bits == 0x00)
        this->season_mode_text_sensor_->publish_state("Winter");
      else if (season_bits == 0x08)
        this->season_mode_text_sensor_->publish_state("Summer");
      else if (season_bits == 0x10)
        this->season_mode_text_sensor_->publish_state("Cooling");
      else
        this->season_mode_text_sensor_->publish_state("Unknown");
    }
#endif

    bool airing_active = (rx_buffer[27] & 0x20) != 0;
    this->update_register_current(REG_AIRING, airing_active ? 1 : 0);

#ifdef USE_BINARY_SENSOR
    if (this->ventilation_mode_binary_sensor_ != nullptr)
    {
      this->ventilation_mode_binary_sensor_->publish_state(airing_active);
    }
#endif

    uint8_t raw_fs = rx_buffer[28] & 0x07;
#ifdef USE_TEXT_SENSOR
    if (this->master_fan_speed_text_sensor_ != nullptr)
    {
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
      this->master_fan_speed_text_sensor_->publish_state(fs_str);
    }
#endif

#ifdef USE_CLIMATE
    if (this->climate_ != nullptr)
    {
      this->climate_->current_temperature = room_temp;
      this->climate_->current_humidity = rx_buffer[23];

      if (!this->managed_registers_[REG_MANUAL_TEMP].has_desired &&
          !this->managed_registers_[REG_THERMOSTAT_MODE].has_desired)
      {
        this->climate_->target_temperature = target_temp;
      }

      if (!this->managed_registers_[REG_SEASON_MODE].has_desired)
      {
        if (season_idx == 0)
        {
          this->climate_->mode = climate::CLIMATE_MODE_HEAT;
        }
        else if (season_idx == 1)
        {
          this->climate_->mode = climate::CLIMATE_MODE_FAN_ONLY;
        }
        else
        {
          this->climate_->mode = climate::CLIMATE_MODE_COOL;
        }
      }

      if (raw_fs <= 1)
      {
        this->climate_->action = climate::CLIMATE_ACTION_IDLE;
      }
      else if (season_idx == 0)
      {
        this->climate_->action = (room_temp < target_temp)
                                     ? climate::CLIMATE_ACTION_HEATING
                                     : climate::CLIMATE_ACTION_FAN;
      }
      else if (season_idx == 1)
      {
        this->climate_->action = climate::CLIMATE_ACTION_FAN;
      }
      else
      {
        this->climate_->action = cooling_demand
                                     ? climate::CLIMATE_ACTION_COOLING
                                     : climate::CLIMATE_ACTION_FAN;
      }

      this->climate_->publish_state();
    }
#endif

    ESP_LOGD(TAG, "Unparsed panel status bytes: [6]=%02X [10]=%02X [11]=%02X",
             rx_buffer[6], rx_buffer[10], rx_buffer[11]);
  }

  void AwentaAurosComponent::handle_master_config_(const std::vector<uint8_t> &rx_buffer)
  {
    float config_intake_temp = decode_value_(rx_buffer[6], rx_buffer[7]);
    float config_comfort_temp = decode_value_(rx_buffer[8], rx_buffer[9]);
    float config_eco_winter_temp = decode_value_(rx_buffer[10], rx_buffer[11]);
    float config_eco_cooling_temp = decode_value_(rx_buffer[12], rx_buffer[13]);
    float config_manual_temp = decode_value_(rx_buffer[14], rx_buffer[15]);
    float config_away_temp = decode_value_(rx_buffer[16], rx_buffer[17]);

    this->update_register_current(REG_COMFORT_TEMP,
                                  (int32_t)decode_index_(rx_buffer[8], rx_buffer[9]) - 2000);
    this->update_register_current(REG_ECO_WINTER_TEMP,
                                  (int32_t)decode_index_(rx_buffer[10], rx_buffer[11]) - 2000);
    this->update_register_current(REG_ECO_COOLING_TEMP,
                                  (int32_t)decode_index_(rx_buffer[12], rx_buffer[13]) - 2000);
    this->update_register_current(REG_MANUAL_TEMP,
                                  (int32_t)decode_index_(rx_buffer[14], rx_buffer[15]) - 2000);
    this->update_register_current(REG_AWAY_TEMP,
                                  (int32_t)decode_index_(rx_buffer[16], rx_buffer[17]) - 2000);

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
    if (this->config_bypass_mode_text_sensor_ != nullptr)
    {
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

    if (this->config_ghe_mode_text_sensor_ != nullptr)
    {
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
    if (this->config_fan_speed_text_sensor_ != nullptr)
    {
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

    if (this->config_thermostat_mode_text_sensor_ != nullptr)
    {
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
             "Unparsed master config bytes: [5]=%02X [18..24]=%02X %02X %02X %02X %02X %02X %02X [26]=%02X [27]=%02X",
             rx_buffer[5], rx_buffer[18], rx_buffer[19], rx_buffer[20], rx_buffer[21],
             rx_buffer[22], rx_buffer[23], rx_buffer[24], rx_buffer[26], rx_buffer[27]);
  }

  void AwentaAurosComponent::handle_aero_status_(const std::vector<uint8_t> &rx_buffer)
  {
    float intake_temp = decode_value_(rx_buffer[10], rx_buffer[11]);
    float supply_temp = decode_value_(rx_buffer[12], rx_buffer[13]);
    float preheater_temp = decode_value_(rx_buffer[14], rx_buffer[15]);
    float supply_target_temp = decode_value_(rx_buffer[18], rx_buffer[19]);

    uint8_t target_fan_speed = rx_buffer[22];
    uint8_t intake_fan_speed = rx_buffer[25];
    uint8_t supply_fan_speed = rx_buffer[26];
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
    if (this->supply_fan_speed_sensor_ != nullptr)
      this->supply_fan_speed_sensor_->publish_state(supply_fan_speed);
    if (this->supply_target_temperature_sensor_ != nullptr)
      this->supply_target_temperature_sensor_->publish_state(supply_target_temp);
#endif

#ifdef USE_BINARY_SENSOR
    if (this->bypass_position_binary_sensor_ != nullptr)
    {
      this->bypass_position_binary_sensor_->publish_state((bypass_pos & 0x20) != 0);
    }
#endif

    ESP_LOGD(TAG,
             "Unparsed aero status bytes: [6..9]=%02X %02X %02X %02X [16..17]=%02X %02X [20..21]=%02X %02X [23..24]=%02X %02X [27..28]=%02X %02X",
             rx_buffer[6], rx_buffer[7], rx_buffer[8], rx_buffer[9], rx_buffer[16], rx_buffer[17],
             rx_buffer[20], rx_buffer[21], rx_buffer[23], rx_buffer[24], rx_buffer[27], rx_buffer[28]);
  }

  void AwentaAurosComponent::handle_aero_secondary_(const std::vector<uint8_t> &rx_buffer)
  {
    float exhaust_temp = decode_value_(rx_buffer[16], rx_buffer[17]);
    float extract_temp = decode_value_(rx_buffer[18], rx_buffer[19]);
    uint8_t extract_fan_speed = rx_buffer[14];
    uint8_t exhaust_fan_speed = rx_buffer[15];

#ifdef USE_SENSOR
    if (this->exhaust_temperature_sensor_ != nullptr)
      this->exhaust_temperature_sensor_->publish_state(exhaust_temp);
    if (this->extract_temperature_sensor_ != nullptr)
      this->extract_temperature_sensor_->publish_state(extract_temp);
    if (this->extract_fan_speed_sensor_ != nullptr)
      this->extract_fan_speed_sensor_->publish_state(extract_fan_speed);
    if (this->exhaust_fan_speed_sensor_ != nullptr)
      this->exhaust_fan_speed_sensor_->publish_state(exhaust_fan_speed);
#endif

    ESP_LOGD(TAG, "Unparsed aero secondary bytes: [5]=%02X [6..13]=%.1f %.1f %.1f %.1f [28]=%02X",
             rx_buffer[5], decode_value_(rx_buffer[6], rx_buffer[7]),
             decode_value_(rx_buffer[8], rx_buffer[9]),
             decode_value_(rx_buffer[10], rx_buffer[11]),
             decode_value_(rx_buffer[12], rx_buffer[13]), rx_buffer[28]);
  }

  void AwentaAurosComponent::handle_slots_(const std::vector<uint8_t> &rx_buffer)
  {
    for (int i = 0; i < 6; i++)
    {
      int index_id_h = 5 + (i * 4);
      int index_id_l = 6 + (i * 4);
      int index_val_h = 7 + (i * 4);
      int index_val_l = 8 + (i * 4);

      if (index_val_l < 29)
      {
        uint8_t h = rx_buffer[index_id_h];
        uint8_t l = rx_buffer[index_id_l];
        uint8_t val_h = rx_buffer[index_val_h];
        uint8_t val_l = rx_buffer[index_val_l];

        if (!(h == 0x00 && l == 0x00 && val_h == 0x00 && val_l == 0x00) &&
            !(h == 0x7E && l == 0x7E && val_h == 0x7E && val_l == 0x7E))
        {
          uint16_t reg_index = decode_index_(h, l);
          int32_t raw_int = (int32_t)decode_index_(val_h, val_l) - 2000;
          float value_actual = raw_int / 10.0f;

          ESP_LOGI(TAG,
                   "  [B:%02d-%02d] | Slot [%5d] (H:%02X L:%02X) -> Value: %02X %02X (int:%d | %.1f)",
                   index_id_h, index_val_l, reg_index, h, l, val_h, val_l, (int)raw_int,
                   value_actual);
        }
      }
    }
  }

  void AwentaAurosComponent::send_c14_rwt_(uint8_t target_addr, uint8_t command,
                                           const std::array<uint16_t, 6> &regs,
                                           const std::array<uint16_t, 6> &d_raw_values,
                                           uint8_t sender_addr, const std::string &probe_label)
  {
    std::vector<uint8_t> frame(30, 0x00);
    frame[0] = target_addr + 128;
    frame[1] = command;
    frame[2] = 0x00;
    frame[3] = sender_addr;
    frame[4] = 0x00;

    for (int i = 0; i < 6; i++)
    {
      int base = 5 + (i * 4);
      encode_index_(regs[i], frame[base], frame[base + 1]);
      encode_index_(d_raw_values[i], frame[base + 2], frame[base + 3]);
    }

    frame[29] = 0x23;
    frame[2] = calculate_c14_checksum_(frame);

    std::string hex_str;
    for (uint8_t b : frame)
    {
      char buf[4];
      snprintf(buf, sizeof(buf), "%02X ", b);
      hex_str += buf;
    }

    ESP_LOGW(TAG_RWT, ">>> TX [%s] cmd='%c' From:%d(0x%02X) -> To:%d(0x%02X) CRC:%02X | Raw: %s",
             probe_label.c_str(), command, sender_addr, sender_addr, target_addr,
             frame[0], frame[2], hex_str.c_str());

    if (this->flow_control_pin_ != nullptr)
    {
      this->flow_control_pin_->digital_write(true);
    }
    delayMicroseconds(1000);
    this->write_array(frame);
    this->flush();
    delayMicroseconds(1000);
    if (this->flow_control_pin_ != nullptr)
    {
      this->flow_control_pin_->digital_write(false);
    }

    while (this->available())
    {
      uint8_t dummy = this->read();
      (void)dummy;
    }
    this->rx_buffer_.clear();
  }

  void AwentaAurosComponent::execute_scheduled_write_()
  {
    std::array<uint16_t, 6> regs = {0, 0, 0, 0, 0, 0};
    std::array<uint16_t, 6> d_raw_values = {2000, 2000, 2000, 2000, 2000, 2000};
    int slot = 0;
    std::string label = "W:";

    for (const auto &kv : this->managed_registers_)
    {
      if (slot >= 6)
        break;
      uint16_t reg_id = kv.first;
      const auto &reg = kv.second;
      if (reg.has_desired && (!reg.has_current || reg.desired_raw != reg.current_raw))
      {
        regs[slot] = reg_id;
        int32_t encoded_val = reg.desired_raw + 2000;
        if (encoded_val < 0)
          encoded_val = 0;
        d_raw_values[slot] = (uint16_t)encoded_val;

        char buf[32];
        snprintf(buf, sizeof(buf), "%u=%d ", reg_id, (int)reg.desired_raw);
        label += buf;
        slot++;
      }
    }

    if (slot == 0)
      return;

    this->send_c14_rwt_(ADDR_MASTER, 'W', regs, d_raw_values, ADDR_INEXT, label);
  }

  void AwentaAurosComponent::handle_rwt_response_(uint8_t command, uint8_t source_address,
                                                  uint8_t target_address, const std::string &hex_str)
  {
    std::string slots_summary;
    if (command == 'w')
    {
      char sbuf[48];
      snprintf(sbuf, sizeof(sbuf), "ACK mask=0x%02X ", this->rx_buffer_[5]);
      slots_summary = sbuf;
    }

    for (int i = 0; i < 6; i++)
    {
      int base = 5 + (i * 4);
      uint16_t reg_id = decode_index_(this->rx_buffer_[base], this->rx_buffer_[base + 1]);
      int32_t raw_int =
          (int32_t)decode_index_(this->rx_buffer_[base + 2], this->rx_buffer_[base + 3]) - 2000;
      float val_f = raw_int / 10.0f;

      if (reg_id != 0 || raw_int != 0)
      {
        char sbuf[48];
        snprintf(sbuf, sizeof(sbuf), "[%u]=%d(%.1f) ", reg_id, (int)raw_int, val_f);
        slots_summary += sbuf;
      }

      if (command == 'r' && reg_id != 0 && raw_int != 9999)
      {
        this->update_register_current(reg_id, raw_int);
      }
    }

    char full_buf[256];
    snprintf(full_buf, sizeof(full_buf), "cmd='%c' %d->%d: %s| Raw: %s",
             command, source_address, target_address, slots_summary.c_str(), hex_str.c_str());

    ESP_LOGW(TAG_RWT, "<<< RWT RESPONSE RECEIVED! %s", full_buf);
  }

  void AwentaAurosComponent::parse_frame_()
  {
    if (this->rx_buffer_.size() != 30)
    {
      return;
    }

    uint8_t received_checksum = this->rx_buffer_[2];
    uint8_t calculated_checksum = calculate_c14_checksum_(this->rx_buffer_);

    if (received_checksum != calculated_checksum)
    {
      ESP_LOGW(TAG, "Checksum error! Received: %02X, expected: %02X",
               received_checksum, calculated_checksum);
      return;
    }

    uint8_t command = this->rx_buffer_[1];
    uint8_t target_address =
        (this->rx_buffer_[0] >= 128) ? (this->rx_buffer_[0] - 128) : this->rx_buffer_[0];
    uint8_t source_address = this->rx_buffer_[3];
    std::string hex_str;
    for (uint8_t b : this->rx_buffer_)
    {
      char buf[4];
      snprintf(buf, sizeof(buf), "%02X ", b);
      hex_str += buf;
    }

    this->print_header_(command, source_address, target_address, received_checksum, hex_str);

    if (command == 'r' || command == 'w' || command == 't' ||
        ((target_address == ADDR_INEXT || target_address == ADDR_PC || target_address == ADDR_LAN) &&
         !(command == 'D' && this->rx_buffer_[4] == 0x7E)))
    {
      this->handle_rwt_response_(command, source_address, target_address, hex_str);
    }

    if (command == 'D' && source_address == ADDR_MASTER && target_address == ADDR_AERO)
    {
      bool fireplace_active = (this->rx_buffer_[28] & 0x40) != 0;
      this->update_register_current(REG_FIREPLACE, fireplace_active ? 1 : 0);
#ifdef USE_BINARY_SENSOR
      if (this->fireplace_mode_binary_sensor_ != nullptr)
      {
        this->fireplace_mode_binary_sensor_->publish_state(fireplace_active);
      }
#endif
    }

    if (command == 'D' && source_address == ADDR_MASTER && target_address == ADDR_INEXT &&
        this->has_pending_writes())
    {
      this->tx_scheduled_ = true;
      this->scheduled_tx_time_ms_ = millis() + 40;
      ESP_LOGW(TAG_RWT,
               "Detected iNext slot (F0 44 from Master 41 -> 112), scheduling pending register write in 40ms");
    }

    if (command == '!')
    {
      if (source_address == ADDR_MASTER && target_address == ADDR_BROADCAST_PRIMARY)
      {
        this->handle_master_status_(this->rx_buffer_);
        return;
      }
      else if (source_address == ADDR_AERO && target_address == ADDR_BROADCAST_PRIMARY)
      {
        this->handle_aero_status_(this->rx_buffer_);
        return;
      }
      else if (source_address == ADDR_AERO && target_address == ADDR_BROADCAST_SECONDARY)
      {
        this->handle_aero_secondary_(this->rx_buffer_);
        return;
      }
      else if (source_address == ADDR_MASTER && target_address == ADDR_BROADCAST_SECONDARY)
      {
        this->handle_master_config_(this->rx_buffer_);
        return;
      }
    }

    if (command == 'R' || command == 'r' || command == 'W' || command == 'w' ||
        command == 'T' || command == 't')
    {
      this->handle_slots_(this->rx_buffer_);
    }
  }

} // namespace esphome::awenta_auros
