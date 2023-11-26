#include "ac_transfer_switch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ac_transfer_switch {

static const char *const TAG = "ac_transfer_switch";

void AcTransferSwitchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Transfer Switch...");

  parent_->add_on_adc_conversion_callback([this](float voltage) { this->process_adc_conversion(voltage); });
  parent_->add_on_period_callback([this](float voltage_rms) { this->process_period(voltage_rms); });

  ESP_LOGCONFIG(TAG, "Transfer Switch setup finished!");
}

void AcTransferSwitchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Transfer Switch:");

  ESP_LOGCONFIG(TAG, "  Backup Switch Delay: %uus", backup_switch_delay_);
  ESP_LOGCONFIG(TAG, "  Backup Switch Voltage Threshold: %.3fV", backup_switch_voltage_threshold_);
  ESP_LOGCONFIG(TAG, "  Primary Switch Delay: %uus", primary_switch_delay_);
  ESP_LOGCONFIG(TAG, "  Primary Switch Voltage RMS Threshold: %.3fV", primary_switch_voltage_rms_threshold_);

  LOG_BINARY_SENSOR("  ", "Power Sensor", power_sensor_);
}

void HOT AcTransferSwitchComponent::set_power_source(PowerSource value) {
  if (!power_source_dedup_.next(value)) {
    return;
  }

  power_source_ = value;
  output_->set_state(value == PowerSource::BACKUP);

  if (power_sensor_) {
    auto state = value == PowerSource::PRIMARY;
    defer("publish_state", [this, state]() { this->power_sensor_->publish_state(state); });
  }
}

void HOT AcTransferSwitchComponent::process_adc_conversion(float voltage) {
  if (power_source_ != PowerSource::BACKUP) {
    auto time = esp_timer_get_time();

    if (!undervoltage_start_ || voltage >= backup_switch_voltage_threshold_) {
      undervoltage_start_ = time;
    }

    if ((time - undervoltage_start_) >= backup_switch_delay_) {
      stable_voltage_start_ = 0;
      set_power_source(PowerSource::BACKUP);
    }
  }
}

void HOT AcTransferSwitchComponent::process_period(float voltage_rms) {
  if (power_source_ != PowerSource::PRIMARY) {
    auto time = esp_timer_get_time();

    if (!stable_voltage_start_ || voltage_rms < primary_switch_voltage_rms_threshold_) {
      stable_voltage_start_ = time;
    }

    if ((time - stable_voltage_start_) >= primary_switch_delay_) {
      undervoltage_start_ = 0;
      set_power_source(PowerSource::PRIMARY);
    }
  }
}

}  // namespace ac_transfer_switch
}  // namespace esphome