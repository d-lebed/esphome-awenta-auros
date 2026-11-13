#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/helpers.h"
#include "../awenta_auros.h"

namespace esphome::awenta_auros {

class AiringSwitch final : public switch_::Switch, public Parented<AwentaAurosComponent> {
 protected:
  void write_state(bool state) override;
};

class FireplaceSwitch final : public switch_::Switch, public Parented<AwentaAurosComponent> {
 protected:
  void write_state(bool state) override;
};

class GheSwitch final : public switch_::Switch, public Parented<AwentaAurosComponent> {
 protected:
  void write_state(bool state) override;
};

}  // namespace esphome::awenta_auros
