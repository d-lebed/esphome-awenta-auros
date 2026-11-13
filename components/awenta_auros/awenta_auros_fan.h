#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"

#ifdef USE_FAN
#include "esphome/components/fan/fan.h"

namespace esphome::awenta_auros {

class AwentaAurosComponent;

class AwentaAurosFan : public fan::Fan, public Component {
 public:
  AwentaAurosFan();

  void set_parent(AwentaAurosComponent *parent) { this->parent_ = parent; }
  void enable_smooth_speed_mode(uint8_t s1_def, uint8_t s2_def, uint8_t s3_def);

  fan::FanTraits get_traits() override;
  void apply_bus_fan_mode(int32_t fs);
  void apply_bus_active_gear(uint8_t active_gear);
  void apply_smooth_speed_state(int32_t percent, int32_t fan_mode, bool airing);

 protected:
  void control(const fan::FanCall &call) override;

  AwentaAurosComponent *parent_{nullptr};
  fan::FanTraits traits_{false, true, false, 3};
  bool smooth_mode_{false};
  uint8_t s1_default_{30};
  uint8_t s2_default_{60};
  uint8_t s3_default_{90};
  uint8_t last_active_gear_{1};
  int32_t last_smooth_speed_{60};
};

}  // namespace esphome::awenta_auros
#endif
