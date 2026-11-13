#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"

#ifdef USE_CLIMATE
#include "esphome/components/climate/climate.h"

namespace esphome::awenta_auros {

class AwentaAurosComponent;

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

}  // namespace esphome::awenta_auros
#endif
