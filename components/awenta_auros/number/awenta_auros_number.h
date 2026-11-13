#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/helpers.h"
#include "../awenta_auros.h"

namespace esphome::awenta_auros {

class ComfortTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class EcoWinterTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class EcoCoolingTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class ManualTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class AwayTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class PreheaterMinTemperatureNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class SupplyFanSpeed1Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class SupplyFanSpeed2Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class SupplyFanSpeed3Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class SupplyFanAiringSpeedNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class ExhaustFanSpeed1Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class ExhaustFanSpeed2Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class ExhaustFanSpeed3Number final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class ExhaustFanAiringSpeedNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

class SmoothFanSpeedNumber final : public number::Number, public Parented<AwentaAurosComponent> {
 protected:
  void control(float value) override;
};

}  // namespace esphome::awenta_auros
