#include "transfer_switch.h"

namespace esphome {
namespace transfer_switch {

static const char *const TAG = "transfer_switch";

void TransferSwitchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Transfer Switch...");

  ac_voltage_->add_on_adc_conversion_callback([this](float voltage) { this->process_adc_conversion(voltage); });
  ac_voltage_->add_on_period_callback([this](float voltage_rms) { this->process_period(voltage_rms); });

  ESP_LOGCONFIG(TAG, "Transfer Switch setup finished!");
}

void TransferSwitchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Transfer Switch:");

  ESP_LOGCONFIG(TAG, "  Instant switch delay: %uus", instant_switch_delay_);
  ESP_LOGCONFIG(TAG, "  Instant Switch Voltage Threshold: %.3fV", instant_switch_voltage_threshold_);
  ESP_LOGCONFIG(TAG, "  Return to Mains Delay: %uus", return_to_mains_delay_);
  ESP_LOGCONFIG(TAG, "  Min Voltage RMS Threshold: %.1fV", min_voltage_rms_threshold_);
}

void TransferSwitchComponent::add_on_state_callback(std::function<void(bool)> &&callback) {
  state_callback_.add(std::move(callback));
}

void TransferSwitchComponent::set_state(bool state) {
  if (!this->state_dedup_.next(state))
    return;

  state_ = state;
  state_callback_.call(state);
}

void HOT TransferSwitchComponent::process_adc_conversion(float voltage) {
  auto time = esp_timer_get_time();

  if (!state_) {
    if (!undervoltage_start_ || voltage >= instant_switch_voltage_threshold_) {
      undervoltage_start_ = time;
    }

    if ((time - undervoltage_start_) >= instant_switch_delay_) {
      stable_voltage_start_ = 0;
      set_state(true);
    }
  }
}

void HOT TransferSwitchComponent::process_period(float voltage_rms) {
  auto time = esp_timer_get_time();
  if (state_) {
    if (!stable_voltage_start_ || voltage_rms < min_voltage_rms_threshold_) {
      stable_voltage_start_ = time;
    }

    if ((time - stable_voltage_start_) >= return_to_mains_delay_) {
      undervoltage_start_ = 0;
      set_state(false);
    }
  }
}

}  // namespace transfer_switch
}  // namespace esphome