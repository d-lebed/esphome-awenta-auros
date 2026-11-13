#include "awenta_auros.h"

namespace esphome::awenta_auros {

void AwentaAurosComponent::setup() {
  this->rx_buffer_.reserve(128);
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
    this->flow_control_pin_->digital_write(false);
  }
}

void AwentaAurosComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Awenta Auros C14 Component:");
  LOG_PIN("  Flow Control Pin: ", this->flow_control_pin_);
}

#ifdef USE_AWENTA_AUROS_CLIMATE
void AwentaAurosComponent::set_climate(AwentaAurosClimate *clim) {
  this->climate_ = clim;
  if (clim != nullptr) {
    clim->set_parent(this);
  }
}
#endif

#ifdef USE_AWENTA_AUROS_FAN
void AwentaAurosComponent::set_fan(AwentaAurosFan *f) {
  this->fan_ = f;
  if (f != nullptr) {
    f->set_parent(this);
    if (this->sfs_enabled_) {
      f->enable_smooth_speed_mode(this->sfs_speed_1_default_, this->sfs_speed_2_default_,
                                  this->sfs_speed_3_default_);
    }
  }
}
#endif

void AwentaAurosComponent::loop() {
  if (this->tx_scheduled_ && (int32_t) (millis() - this->scheduled_tx_time_ms_) >= 0) {
    this->tx_scheduled_ = false;
    this->execute_scheduled_write_();
  }

  while (this->available()) {
    uint8_t byte = this->read();

    if (byte == 0xFF) {
      continue;
    }

    if (byte >= 0x80) {
      if (!this->rx_buffer_.empty()) {
        ESP_LOGD(TAG, "Resync on new frame header 0x%02X (discarded %d incomplete bytes)",
                 byte, (int) this->rx_buffer_.size());
        this->rx_buffer_.clear();
      }
      this->rx_buffer_.push_back(byte);
      continue;
    }

    if (this->rx_buffer_.empty()) {
      continue;
    }

    this->rx_buffer_.push_back(byte);

    if (this->rx_buffer_.size() == 30) {
      if (this->rx_buffer_[29] == 0x23) {
        this->parse_frame_();
      } else {
        ESP_LOGW(TAG, "30-byte buffer did not end with 0x23 (got 0x%02X), discarding.",
                 this->rx_buffer_[29]);
      }
      this->rx_buffer_.clear();
    }
  }
}

}  // namespace esphome::awenta_auros
