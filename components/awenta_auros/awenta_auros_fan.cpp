#include "awenta_auros_fan.h"

#ifdef USE_FAN
#include <cstring>
#include "awenta_auros.h"

namespace esphome::awenta_auros {

AwentaAurosFan::AwentaAurosFan() {
  this->set_supported_preset_modes({"Auto", "Party"});
}

void AwentaAurosFan::enable_smooth_speed_mode(uint8_t s1_def, uint8_t s2_def, uint8_t s3_def) {
  this->smooth_mode_ = true;
  this->s1_default_ = s1_def;
  this->s2_default_ = s2_def;
  this->s3_default_ = s3_def;
  this->traits_ = fan::FanTraits(false, true, false, 100);
  this->set_supported_preset_modes({"Speed 1", "Speed 2", "Speed 3", "Airing", "Auto", "Party"});
}

fan::FanTraits AwentaAurosFan::get_traits() {
  this->wire_preset_modes_(this->traits_);
  return this->traits_;
}

void AwentaAurosFan::apply_bus_fan_mode(int32_t fs) {
  if (this->smooth_mode_) {
    switch (fs) {
      case 0:  // Off
        this->state = false;
        this->clear_preset_mode_();
        break;
      case 1:  // Speed 1
        this->state = true;
        this->set_preset_mode_("Speed 1");
        break;
      case 2:  // Speed 2
        this->state = true;
        this->set_preset_mode_("Speed 2");
        break;
      case 3:  // Speed 3
        this->state = true;
        this->set_preset_mode_("Speed 3");
        break;
      case 4:  // Party
        this->state = true;
        this->set_preset_mode_("Party");
        break;
      case 5:  // Schedule / Auto
        this->state = true;
        this->set_preset_mode_("Auto");
        break;
      default:
        break;
    }
    return;
  }

  switch (fs) {
    case 0:  // Off
      this->state = false;
      this->clear_preset_mode_();
      break;
    case 1:  // Speed 1
      this->state = true;
      this->speed = 1;
      this->clear_preset_mode_();
      break;
    case 2:  // Speed 2
      this->state = true;
      this->speed = 2;
      this->clear_preset_mode_();
      break;
    case 3:  // Speed 3
      this->state = true;
      this->speed = 3;
      this->clear_preset_mode_();
      break;
    case 4:  // Party
      this->state = true;
      if (this->speed <= 0)
        this->speed = this->last_active_gear_;
      this->set_preset_mode_("Party");
      break;
    case 5:  // Schedule / Auto
      this->state = true;
      if (this->speed <= 0)
        this->speed = this->last_active_gear_;
      this->set_preset_mode_("Auto");
      break;
    default:
      break;
  }
}

void AwentaAurosFan::apply_bus_active_gear(uint8_t active_gear) {
  if (active_gear >= 1 && active_gear <= 3) {
    this->last_active_gear_ = active_gear;
    if (!this->smooth_mode_ && this->has_preset_mode()) {
      this->speed = active_gear;
    }
  }
}

void AwentaAurosFan::apply_smooth_speed_state(int32_t percent, int32_t fan_mode, bool airing) {
  if (!this->smooth_mode_)
    return;

  if (airing || percent >= 100) {
    this->state = true;
    this->speed = 100;
    this->last_smooth_speed_ = 100;
    this->set_preset_mode_("Airing");
    this->publish_state();
    return;
  }

  if (percent <= 0 || fan_mode == 0) {
    this->state = false;
    this->clear_preset_mode_();
    this->publish_state();
    return;
  }

  this->state = true;
  this->speed = percent;
  this->last_smooth_speed_ = percent;

  if (fan_mode == 4) {
    this->set_preset_mode_("Party");
  } else if (fan_mode == 5) {
    this->set_preset_mode_("Auto");
  } else if (fan_mode == 1 && percent == (int32_t) this->s1_default_) {
    this->set_preset_mode_("Speed 1");
  } else if (fan_mode == 2 && percent == (int32_t) this->s2_default_) {
    this->set_preset_mode_("Speed 2");
  } else if (fan_mode == 3 && percent == (int32_t) this->s3_default_) {
    this->set_preset_mode_("Speed 3");
  } else {
    this->clear_preset_mode_();
  }
  this->publish_state();
}

void AwentaAurosFan::control(const fan::FanCall &call) {
  if (call.has_preset_mode()) {
    const char *preset = call.get_preset_mode();
    this->state = true;
    this->set_preset_mode_(preset);
    if (this->parent_ != nullptr) {
      if (strcmp(preset, "Speed 1") == 0) {
        this->speed = this->smooth_mode_ ? this->s1_default_ : 1;
        this->parent_->set_desired_fan_mode_index(1);
      } else if (strcmp(preset, "Speed 2") == 0) {
        this->speed = this->smooth_mode_ ? this->s2_default_ : 2;
        this->parent_->set_desired_fan_mode_index(2);
      } else if (strcmp(preset, "Speed 3") == 0) {
        this->speed = this->smooth_mode_ ? this->s3_default_ : 3;
        this->parent_->set_desired_fan_mode_index(3);
      } else if (strcmp(preset, "Airing") == 0) {
        if (this->smooth_mode_) {
          this->speed = 100;
          this->parent_->set_desired_smooth_fan_speed(100.0f);
        } else {
          this->parent_->set_desired_airing(true);
        }
      } else if (strcmp(preset, "Auto") == 0) {
        this->parent_->set_desired_fan_mode_index(5);  // Schedule
      } else if (strcmp(preset, "Party") == 0) {
        this->parent_->set_desired_fan_mode_index(4);  // Party
      }
    }
    this->publish_state();
    return;
  }

  if (call.get_state().has_value() && !*call.get_state()) {
    this->state = false;
    this->clear_preset_mode_();
    if (this->parent_ != nullptr) {
      if (this->smooth_mode_) {
        this->parent_->set_desired_smooth_fan_speed(0.0f);
      } else {
        this->parent_->set_desired_fan_mode_index(0);  // Off
      }
    }
    this->publish_state();
    return;
  }

  if (call.get_speed().has_value()) {
    int spd = *call.get_speed();
    if (this->smooth_mode_) {
      if (spd <= 0) {
        this->state = false;
        this->clear_preset_mode_();
        if (this->parent_ != nullptr) {
          this->parent_->set_desired_smooth_fan_speed(0.0f);
        }
      } else {
        if (spd > 100)
          spd = 100;
        this->state = true;
        this->speed = spd;
        if (this->parent_ != nullptr) {
          this->parent_->set_desired_smooth_fan_speed((float) spd);
        }
      }
      this->publish_state();
      return;
    }

    if (spd <= 0) {
      this->state = false;
      this->clear_preset_mode_();
      if (this->parent_ != nullptr) {
        this->parent_->set_desired_fan_mode_index(0);
      }
    } else {
      if (spd > 3)
        spd = 3;
      this->state = true;
      this->speed = spd;
      this->clear_preset_mode_();
      if (this->parent_ != nullptr) {
        this->parent_->set_desired_fan_mode_index((size_t) spd);
      }
    }
    this->publish_state();
    return;
  }

  if (call.get_state().has_value() && *call.get_state()) {
    this->state = true;
    if (!this->has_preset_mode()) {
      this->set_preset_mode_("Auto");
      if (this->parent_ != nullptr) {
        this->parent_->set_desired_fan_mode_index(5);  // Schedule
      }
    }
    this->publish_state();
  }
}

}  // namespace esphome::awenta_auros
#endif
