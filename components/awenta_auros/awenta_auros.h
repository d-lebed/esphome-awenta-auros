#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#ifdef USE_SELECT
#include "esphome/components/select/select.h"
#endif
#ifdef USE_SWITCH
#include "esphome/components/switch/switch.h"
#endif
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"
#endif

#include <array>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace esphome::awenta_auros {

class AwentaAurosComponent;

#ifdef USE_CLIMATE
class AwentaAurosClimate : public climate::Climate, public Component {
 public:
  AwentaAurosClimate();

  void set_parent(AwentaAurosComponent *parent) { this->parent_ = parent; }

  void apply_bus_fan_mode(int32_t fs);
  void apply_bus_preset(int32_t therm);

 protected:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;

  AwentaAurosComponent *parent_{nullptr};
};
#endif

class AwentaAurosComponent : public Component, public uart::UARTDevice {
#ifdef USE_SENSOR
  SUB_SENSOR(target_temperature)
  SUB_SENSOR(room_temperature)
  SUB_SENSOR(room_humidity)
  SUB_SENSOR(intake_temperature)
  SUB_SENSOR(supply_temperature)
  SUB_SENSOR(preheater_temperature)
  SUB_SENSOR(supply_target_temperature)
  SUB_SENSOR(exhaust_temperature)
  SUB_SENSOR(extract_temperature)
  SUB_SENSOR(target_fan_speed)
  SUB_SENSOR(intake_fan_speed)
  SUB_SENSOR(supply_fan_speed)
  SUB_SENSOR(extract_fan_speed)
  SUB_SENSOR(exhaust_fan_speed)
  SUB_SENSOR(config_intake_temperature)
  SUB_SENSOR(config_comfort_temperature)
  SUB_SENSOR(config_eco_winter_temperature)
  SUB_SENSOR(config_eco_cooling_temperature)
  SUB_SENSOR(config_manual_temperature)
  SUB_SENSOR(config_away_temperature)
#endif

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(cooling_demand)
  SUB_BINARY_SENSOR(ventilation_mode)
  SUB_BINARY_SENSOR(bypass_position)
  SUB_BINARY_SENSOR(away)
  SUB_BINARY_SENSOR(fireplace_mode)
#endif

#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(master_fan_speed)
  SUB_TEXT_SENSOR(season_mode)
  SUB_TEXT_SENSOR(thermostat_mode)
  SUB_TEXT_SENSOR(config_fan_speed)
  SUB_TEXT_SENSOR(config_thermostat_mode)
  SUB_TEXT_SENSOR(config_bypass_mode)
  SUB_TEXT_SENSOR(config_ghe_mode)
  SUB_TEXT_SENSOR(time)
  SUB_TEXT_SENSOR(day_of_week)
#endif

#ifdef USE_SELECT
  SUB_SELECT(fan_mode)
  SUB_SELECT(thermostat_mode)
  SUB_SELECT(bypass_mode)
#endif

#ifdef USE_SWITCH
  SUB_SWITCH(airing)
  SUB_SWITCH(fireplace)
  SUB_SWITCH(ghe)
#endif

#ifdef USE_NUMBER
  SUB_NUMBER(comfort_temperature)
  SUB_NUMBER(eco_winter_temperature)
  SUB_NUMBER(eco_cooling_temperature)
  SUB_NUMBER(manual_temperature)
  SUB_NUMBER(away_temperature)
#endif

 public:
  static constexpr uint8_t ADDR_MASTER = 41;
  static constexpr uint8_t ADDR_NANO_2 = 42;
  static constexpr uint8_t ADDR_AERO = 99;
  static constexpr uint8_t ADDR_BROADCAST_PRIMARY = 100;
  static constexpr uint8_t ADDR_BROADCAST_SECONDARY = 101;
  static constexpr uint8_t ADDR_LAN = 111;
  static constexpr uint8_t ADDR_INEXT = 112;
  static constexpr uint8_t ADDR_PC = 113;

  // NANO COLOR 2 C14 register indices (R/W)
  static constexpr uint16_t REG_COMFORT_TEMP = 3;
  static constexpr uint16_t REG_ECO_WINTER_TEMP = 4;
  static constexpr uint16_t REG_ECO_COOLING_TEMP = 5;
  static constexpr uint16_t REG_MANUAL_TEMP = 6;
  static constexpr uint16_t REG_AWAY_TEMP = 7;
  static constexpr uint16_t REG_SEASON_MODE = 64;
  static constexpr uint16_t REG_FIREPLACE = 76;
  static constexpr uint16_t REG_GHE = 78;
  static constexpr uint16_t REG_BYPASS_MODE = 79;
  static constexpr uint16_t REG_FAN_MODE = 80;
  static constexpr uint16_t REG_AIRING = 81;
  static constexpr uint16_t REG_THERMOSTAT_MODE = 88;

  struct ManagedRegister {
    int32_t current_raw{0};
    bool has_current{false};
    int32_t desired_raw{0};
    bool has_desired{false};
  };

  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_flow_control_pin(GPIOPin *pin) { this->flow_control_pin_ = pin; }
#ifdef USE_TIME
  void set_time(time::RealTimeClock *time) { this->time_ = time; }
#endif
#ifdef USE_CLIMATE
  void set_climate(AwentaAurosClimate *clim) {
    this->climate_ = clim;
    if (clim != nullptr) {
      clim->set_parent(this);
    }
  }
#endif

  static std::string fan_mode_to_string(int32_t mode);
  static int32_t fan_mode_from_string(const std::string &option);
  static std::string thermostat_mode_to_string(int32_t mode);
  static int32_t thermostat_mode_from_string(const std::string &option);
  static std::string bypass_mode_to_string(int32_t mode);
  static int32_t bypass_mode_from_string(const std::string &option);

  void set_desired_register(uint16_t reg_id, int32_t desired_raw);
  void update_register_current(uint16_t reg_id, int32_t current_raw);
  bool has_pending_writes() const;

  void set_desired_fan_mode(const std::string &option);
  void set_desired_fan_mode_index(size_t index);
  void set_desired_thermostat_mode(const std::string &option);
  void set_desired_thermostat_mode_index(size_t index);
  void set_desired_bypass_mode(const std::string &option);
  void set_desired_bypass_mode_index(size_t index);
  void set_desired_airing(bool enabled);
  void set_desired_fireplace(bool enabled);
  void set_desired_ghe(bool enabled);
  void set_desired_season_mode(int32_t season_mode);
  void set_desired_temperature(uint16_t reg_id, float temp_c);
  void set_climate_target_temperature(float temp_c);

 protected:
  static float decode_value_(uint8_t h, uint8_t l);
  static void encode_value_(float value, uint8_t &h, uint8_t &l);
  static uint16_t decode_index_(uint8_t h, uint8_t l);
  static void encode_index_(uint16_t index, uint8_t &h, uint8_t &l);
  static uint8_t calculate_c14_checksum_(const std::vector<uint8_t> &frame);

  void parse_frame_();
  void print_header_(uint8_t command, uint8_t source_address, uint8_t target_address,
                     uint8_t checksum, const std::string &hex_str);
  void handle_master_status_(const std::vector<uint8_t> &rx_buffer);
  void handle_master_config_(const std::vector<uint8_t> &rx_buffer);
  void handle_aero_status_(const std::vector<uint8_t> &rx_buffer);
  void handle_aero_secondary_(const std::vector<uint8_t> &rx_buffer);
  void handle_slots_(const std::vector<uint8_t> &rx_buffer);
  void handle_rwt_response_(uint8_t command, uint8_t source_address,
                            uint8_t target_address, const std::string &hex_str);

  void send_c14_rwt_(uint8_t target_addr, uint8_t command,
                     const std::array<uint16_t, 6> &regs,
                     const std::array<uint16_t, 6> &d_raw_values,
                     uint8_t sender_addr, const std::string &probe_label);
  void execute_scheduled_write_();

  std::vector<uint8_t> rx_buffer_;
  std::map<uint16_t, ManagedRegister> managed_registers_;

  GPIOPin *flow_control_pin_{nullptr};
#ifdef USE_TIME
  time::RealTimeClock *time_{nullptr};
#endif
#ifdef USE_CLIMATE
  AwentaAurosClimate *climate_{nullptr};
#endif

  bool tx_scheduled_{false};
  uint32_t scheduled_tx_time_ms_{0};
  bool is_away_active_{false};

  uint16_t current_year_{2026};
  uint8_t current_month_{1};
  uint8_t current_day_{1};
  uint8_t current_hour_{0};
  uint8_t current_minute_{0};
};

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
    this->parent_->set_desired_temperature(AwentaAurosComponent::REG_COMFORT_TEMP, value);
  }
};

class EcoWinterTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(AwentaAurosComponent::REG_ECO_WINTER_TEMP, value);
  }
};

class EcoCoolingTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(AwentaAurosComponent::REG_ECO_COOLING_TEMP, value);
  }
};

class ManualTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(AwentaAurosComponent::REG_MANUAL_TEMP, value);
  }
};

class AwayTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override {
    this->publish_state(value);
    this->parent_->set_desired_temperature(AwentaAurosComponent::REG_AWAY_TEMP, value);
  }
};
#endif

}  // namespace esphome::awenta_auros
