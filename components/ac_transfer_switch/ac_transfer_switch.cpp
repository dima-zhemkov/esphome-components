#include "ac_transfer_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ac_transfer_switch {

static const char *const TAG = "ac_transfer_switch";

void AcTransferSwitchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AC Transfer Switch...");

  this->output_->set_state(this->power_source_ == PowerSource::BACKUP);

  if (this->power_sensor_) {
    this->power_sensor_->publish_state(this->power_source_ == PowerSource::PRIMARY);
  }

  this->parent_->add_on_adc_conversion_callback([this](float voltage) { this->process_adc_conversion(voltage); });
  this->parent_->add_on_period_callback([this](float voltage_rms) { this->process_period(voltage_rms); });

  ESP_LOGCONFIG(TAG, "AC Transfer Switch setup finished!");
}

void AcTransferSwitchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "AC Transfer Switch:");

  ESP_LOGCONFIG(TAG, "  Backup Switch Delay: %uus", this->backup_switch_delay_);
  ESP_LOGCONFIG(TAG, "  Backup Switch Voltage Threshold: %.3fV", this->backup_switch_voltage_threshold_);
  ESP_LOGCONFIG(TAG, "  Primary Switch Delay: %uus", this->primary_switch_delay_);
  ESP_LOGCONFIG(TAG, "  Primary Switch Voltage RMS Threshold: %.3fV", this->primary_switch_voltage_rms_threshold_);

  LOG_BINARY_SENSOR("  ", "Power Sensor", this->power_sensor_);
}

void HOT AcTransferSwitchComponent::set_power_source(PowerSource value) {
  if (!this->power_source_dedup_.next(value)) {
    return;
  }

  this->power_source_ = value;

  this->output_->set_state(this->power_source_ == PowerSource::BACKUP);

  if (this->power_sensor_) {
    auto state = this->power_source_ == PowerSource::PRIMARY;
    defer("publish_state", [this, state]() { this->power_sensor_->publish_state(state); });
  }
}

void HOT AcTransferSwitchComponent::process_adc_conversion(float voltage) {
  if (this->power_source_ != PowerSource::BACKUP) {
    auto time = esp_timer_get_time();

    if (!this->undervoltage_start_ || voltage >= this->backup_switch_voltage_threshold_) {
      this->undervoltage_start_ = time;
    }

    if ((time - this->undervoltage_start_) >= this->backup_switch_delay_) {
      this->stable_voltage_start_ = 0;
      this->set_power_source(PowerSource::BACKUP);
    }
  }
}

void HOT AcTransferSwitchComponent::process_period(float voltage_rms) {
  if (this->power_source_ != PowerSource::PRIMARY) {
    auto time = esp_timer_get_time();

    if (!this->stable_voltage_start_ || voltage_rms < this->primary_switch_voltage_rms_threshold_) {
      this->stable_voltage_start_ = time;
    }

    if ((time - this->stable_voltage_start_) >= this->primary_switch_delay_) {
      this->undervoltage_start_ = 0;
      this->set_power_source(PowerSource::PRIMARY);
    }
  }
}

}  // namespace ac_transfer_switch
}  // namespace esphome