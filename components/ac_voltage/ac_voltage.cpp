#include "ac_voltage.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ac_voltage {

static const char *const TAG = "ac_voltage";

void AcVoltageSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AC Voltage sensor...");

  auto period = ((uint64_t) 1000000) / mains_frequency_;

  timer_ = new esp_timer_handle_t;
  esp_timer_create_args_t timer_args = {
      .callback = &AcVoltageSensor::timer_callback,
      .arg = static_cast<void *>(this),
      .dispatch_method = ESP_TIMER_TASK,
      .name = "ac_voltage_period_timer",
      .skip_unhandled_events = true,
  };
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, timer_));
  ESP_ERROR_CHECK(esp_timer_start_periodic(*timer_, period));

  parent_->add_on_conversion_callback([this](float sample) { this->process_adc_conversion(sample); });

  ESP_LOGCONFIG(TAG, "AC Voltage sensor setup finished!");
}

void AcVoltageSensor::dump_config() {
  LOG_SENSOR("", "AC Voltage", this);

  ESP_LOGCONFIG(TAG, "  Multiplier: %.3f", multiplier_);
  ESP_LOGCONFIG(TAG, "  Midpoint: %.1fV", midpoint_);
  ESP_LOGCONFIG(TAG, "  Mains Frequency: %uHz", mains_frequency_);

  LOG_UPDATE_INTERVAL(this);
}

void AcVoltageSensor::update() {
  auto voltage = sample();
  ESP_LOGV(TAG, "'%s': Got RMS voltage=%.4fV", get_name().c_str(), voltage);
  publish_state(voltage);
}

float AcVoltageSensor::sample() { return voltage_rms_; }

void AcVoltageSensor::add_on_adc_conversion_callback(std::function<void(float)> &&callback) {
  adc_conversion_callback_.add(std::move(callback));
}

void AcVoltageSensor::add_on_period_callback(std::function<void(float)> &&callback) {
  period_callback_.add(std::move(callback));
}

void HOT AcVoltageSensor::process_adc_conversion(float sample) {
  auto voltage = abs((sample * multiplier_) - midpoint_);
  auto time = esp_timer_get_time();

  adc_conversion_callback_(voltage);

  taskENTER_CRITICAL(&voltage_sum_spinlock_);
  voltage_samples_squared_sum_ += voltage * voltage;
  voltage_samples_count_++;
  taskEXIT_CRITICAL(&voltage_sum_spinlock_);
}

void HOT AcVoltageSensor::measure_voltage() {
  taskENTER_CRITICAL(&voltage_sum_spinlock_);
  auto sum = voltage_samples_squared_sum_;
  auto count = voltage_samples_count_;
  voltage_samples_squared_sum_ = 0;
  voltage_samples_count_ = 0;
  taskEXIT_CRITICAL(&voltage_sum_spinlock_);

  voltage_rms_ = sqrtf(sum / count);

  period_callback_.call(voltage_rms_);

#ifdef ESPHOME_LOG_HAS_VERBOSE
  auto time = esp_timer_get_time();
  min_period_samples_ = std::min(min_period_samples_, count);
  max_period_samples_ = std::max(max_period_samples_, count);
  if (time % 1000000 < 20000) {
    auto min = min_period_samples_;
    auto max = max_period_samples_;
    auto voltage = voltage_rms_;
    defer([count, min, max, voltage]() {
      ESP_LOGV(TAG, "Count: %u, Min count: %u, Max count: %u, V RMS: %f", count, min, max, voltage);
    });
    min_period_samples_ = std::numeric_limits<uint32_t>::max();
    max_period_samples_ = std::numeric_limits<uint32_t>::min();
  }
#endif
}

void HOT AcVoltageSensor::timer_callback(void *arg) {
  AcVoltageSensor *ac_voltage = static_cast<AcVoltageSensor *>(arg);
  ac_voltage->measure_voltage();
}

}  // namespace ac_voltage
}  // namespace esphome