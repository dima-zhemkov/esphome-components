#ifdef USE_ESP32

#include "transfer_switch.h"

namespace esphome {
namespace transfer_switch {

static const char *const TAG = "transfer_switch";

void TransferSwitchComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Transfer Switch...");

  uint64_t period = frequency_to_microseconds(mains_frequency_);

  timer_ = new esp_timer_handle_t;
  esp_timer_create_args_t timer_args = {
      .callback = &TransferSwitchComponent::timer_callback,
      .arg = static_cast<void *>(this),
      .dispatch_method = ESP_TIMER_TASK,
      .name = "transfer_switch_timer",
      .skip_unhandled_events = true,
  };
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, timer_));
  ESP_ERROR_CHECK(esp_timer_start_periodic(*timer_, period));

  parent_->add_on_sample_callback([this](float sample) { this->process_sample(sample); });

  ESP_LOGCONFIG(TAG, "Transfer Switch setup finished!");
}

void TransferSwitchComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Transfer Switch:");

  ESP_LOGCONFIG(TAG, "  Multiplier: %.3f", multiplier_);
  ESP_LOGCONFIG(TAG, "  Midpoint: %.1fV", midpoint_);
  ESP_LOGCONFIG(TAG, "  Undervoltage Delay: %uus", undervoltage_delay_);
  ESP_LOGCONFIG(TAG, "  Return to Mains Delay: %uus", return_to_mains_delay_);
  ESP_LOGCONFIG(TAG, "  Min Voltage RMS Threshold: %.1fV", min_voltage_rms_threshold_);
  ESP_LOGCONFIG(TAG, "  Mains Frequency: %uHz", mains_frequency_);
  ESP_LOGCONFIG(TAG, "  Undervoltage Threshold: %.3fV", undervoltage_threshold_);
}

void TransferSwitchComponent::add_on_state_callback(std::function<void(bool)> &&callback) {
  state_callback_.add(std::move(callback));
}

void TransferSwitchComponent::add_on_rms_voltage_callback(std::function<void(float)> &&callback) {
  rms_voltage_callback_.add(std::move(callback));
}

void TransferSwitchComponent::set_state(bool state) {
  if (!this->state_dedup_.next(state))
    return;

  state_ = state;
  state_callback_.call(state);
}

void TransferSwitchComponent::process_sample(float sample) {
  auto voltage = abs((sample * multiplier_) - midpoint_);
  auto time = esp_timer_get_time();

  if (!state_) {
    if (!undervoltage_start_ || voltage >= undervoltage_threshold_) {
      undervoltage_start_ = time;
    }

    if ((time - undervoltage_start_) >= undervoltage_delay_) {
      stable_voltage_start_ = 0;
      set_state(true);
    }
  }

#ifdef ESPHOME_LOG_HAS_VERBOSE
  min_sample_ = std::min(min_sample_, sample);
  max_sample_ = std::max(max_sample_, sample);
#endif

  taskENTER_CRITICAL(&voltage_sum_spinlock_);
  voltage_samples_squared_sum_ += voltage * voltage;
  voltage_samples_count_++;
  taskEXIT_CRITICAL(&voltage_sum_spinlock_);
}

void TransferSwitchComponent::measure_voltage() {
  taskENTER_CRITICAL(&voltage_sum_spinlock_);
  auto sum = voltage_samples_squared_sum_;
  auto count = voltage_samples_count_;
  voltage_samples_squared_sum_ = 0;
  voltage_samples_count_ = 0;
  taskEXIT_CRITICAL(&voltage_sum_spinlock_);

  voltage_rms_ = sqrtf(sum / count);

  auto time = esp_timer_get_time();
  if (state_) {
    if (!stable_voltage_start_ || voltage_rms_ < min_voltage_rms_threshold_) {
      stable_voltage_start_ = time;
    }

    if ((time - stable_voltage_start_) >= return_to_mains_delay_) {
      undervoltage_start_ = 0;
      set_state(false);
    }
  }

  rms_voltage_callback_.call(voltage_rms_);

#ifdef ESPHOME_LOG_HAS_VERBOSE
  min_period_samples_ = std::min(min_period_samples_, count);
  max_period_samples_ = std::max(max_period_samples_, count);
  if (time % 1000000 < 20000) {
    ESP_LOGV(TAG, "Count: %u, Min count: %u, Max count: %u, V RMS: %f, Min V sample: %f, Max V sample: %f", count,
             min_period_samples_, max_period_samples_, voltage_rms_, min_sample_, max_sample_);
    min_period_samples_ = std::numeric_limits<uint32_t>::max();
    max_period_samples_ = std::numeric_limits<uint32_t>::min();
    min_sample_ = std::numeric_limits<float>::max();
    max_sample_ = std::numeric_limits<float>::min();
  }
#endif
}

int64_t TransferSwitchComponent::frequency_to_microseconds(uint32_t frequency) {
  return ((int64_t) 1000000) / frequency;
}

void TransferSwitchComponent::timer_callback(void *arg) {
  TransferSwitchComponent *transfer_switch = static_cast<TransferSwitchComponent *>(arg);
  transfer_switch->measure_voltage();
}

}  // namespace transfer_switch
}  // namespace esphome

#endif