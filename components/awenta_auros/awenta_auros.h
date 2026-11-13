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

#include <array>
#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "c14_protocol.h"
#ifdef USE_AWENTA_AUROS_CLIMATE
#include "climate/awenta_auros_climate.h"
#endif
#ifdef USE_AWENTA_AUROS_FAN
#include "fan/awenta_auros_fan.h"
#endif

namespace esphome::awenta_auros {

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
  SUB_SENSOR(supply_efficiency)
  SUB_SENSOR(exhaust_efficiency)
  SUB_SENSOR(config_intake_temperature)
  SUB_SENSOR(config_comfort_temperature)
  SUB_SENSOR(config_eco_winter_temperature)
  SUB_SENSOR(config_eco_cooling_temperature)
  SUB_SENSOR(config_manual_temperature)
  SUB_SENSOR(config_away_temperature)
  SUB_SENSOR(config_preheater_min_temperature)
  SUB_SENSOR(config_supply_fan_speed_1)
  SUB_SENSOR(config_supply_fan_speed_2)
  SUB_SENSOR(config_supply_fan_speed_3)
  SUB_SENSOR(config_supply_fan_airing_speed)
  SUB_SENSOR(config_exhaust_fan_speed_1)
  SUB_SENSOR(config_exhaust_fan_speed_2)
  SUB_SENSOR(config_exhaust_fan_speed_3)
  SUB_SENSOR(config_exhaust_fan_airing_speed)
#endif

#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(cooling_demand)
  SUB_BINARY_SENSOR(bypass_position)
  SUB_BINARY_SENSOR(schedule_mode)
  SUB_BINARY_SENSOR(config_airing)
  SUB_BINARY_SENSOR(config_away)
  SUB_BINARY_SENSOR(config_fireplace_mode)
#endif

#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(active_fan_speed)
  SUB_TEXT_SENSOR(thermostat_mode)
  SUB_TEXT_SENSOR(config_season_mode)
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
  SUB_NUMBER(preheater_min_temperature)
  SUB_NUMBER(supply_fan_speed_1)
  SUB_NUMBER(supply_fan_speed_2)
  SUB_NUMBER(supply_fan_speed_3)
  SUB_NUMBER(supply_fan_airing_speed)
  SUB_NUMBER(exhaust_fan_speed_1)
  SUB_NUMBER(exhaust_fan_speed_2)
  SUB_NUMBER(exhaust_fan_speed_3)
  SUB_NUMBER(exhaust_fan_airing_speed)
  SUB_NUMBER(smooth_fan_speed)
#endif

 public:
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
#ifdef USE_AWENTA_AUROS_CLIMATE
  void set_climate(AwentaAurosClimate *clim);
#endif
#ifdef USE_AWENTA_AUROS_FAN
  void set_fan(AwentaAurosFan *f);
#endif

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
  void set_desired_preheater_min_temperature(float temp_c);
  void set_desired_fan_speed_percent(uint16_t reg_id, float percent);
  void set_climate_target_temperature(float temp_c);

  void configure_smooth_fan_speed(uint8_t min_speed, uint8_t max_speed,
                                  uint8_t speed_1_default, uint8_t speed_2_default,
                                  uint8_t speed_3_default);
  void set_desired_smooth_fan_speed(float percent);
  void reset_sfs_fan_speeds_to_defaults();

 protected:
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
  void publish_fan_speed_numbers_(int32_t s1, int32_t s2, int32_t s3);
  void update_smooth_fan_speed_from_bus_();
  void publish_efficiency_sensors_();

  std::vector<uint8_t> rx_buffer_;
  std::map<uint16_t, ManagedRegister> managed_registers_;

  bool has_primary_temps_{false};
  bool has_secondary_temps_{false};
  float last_preheater_temp_{0.0f};
  float last_supply_temp_{0.0f};
  float last_extract_temp_{0.0f};
  float last_exhaust_temp_{0.0f};

  bool sfs_enabled_{false};
  uint8_t sfs_min_speed_{1};
  uint8_t sfs_max_speed_{99};
  uint8_t sfs_speed_1_default_{30};
  uint8_t sfs_speed_2_default_{60};
  uint8_t sfs_speed_3_default_{90};
  uint8_t last_bus_active_gear_{0};

  GPIOPin *flow_control_pin_{nullptr};
#ifdef USE_TIME
  time::RealTimeClock *time_{nullptr};
#endif
#ifdef USE_AWENTA_AUROS_CLIMATE
  AwentaAurosClimate *climate_{nullptr};
#endif
#ifdef USE_AWENTA_AUROS_FAN
  AwentaAurosFan *fan_{nullptr};
#endif

  bool tx_scheduled_{false};
  uint32_t scheduled_tx_time_ms_{0};
  uint32_t inext_window_start_ms_{0};
  bool is_away_active_{false};

  uint16_t current_year_{2026};
  uint8_t current_month_{1};
  uint8_t current_day_{1};
  uint8_t current_hour_{0};
  uint8_t current_minute_{0};
};

}  // namespace esphome::awenta_auros
