#pragma once

#include "esphome/components/select/select.h"
#include "esphome/core/helpers.h"
#include "../awenta_auros.h"

namespace esphome::awenta_auros {

class FanModeSelect final : public select::Select, public Parented<AwentaAurosComponent> {
 protected:
  void control(size_t index) override;
};

class ThermostatModeSelect final : public select::Select, public Parented<AwentaAurosComponent> {
 protected:
  void control(size_t index) override;
};

class BypassModeSelect final : public select::Select, public Parented<AwentaAurosComponent> {
 protected:
  void control(size_t index) override;
};

}  // namespace esphome::awenta_auros
