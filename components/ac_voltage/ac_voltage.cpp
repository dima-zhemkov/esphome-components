#include "ac_voltage.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ac_voltage {

static const char *const TAG = "ac_voltage";

void AcVoltageSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AC Voltage sensor...");

  auto period = ((uint64_t) 1000000) / this->mains_frequency_;

  this->timer_ = new esp_timer_handle_t;
  esp_timer_create_args_t timer_args = {
      .callback = &AcVoltageSensor::timer_callback,
      .arg = static_cast<void *>(this),
      .dispatch_method = ESP_TIMER_TASK,
      .name = "ac_voltage_period_timer",
      .skip_unhandled_events = true,
  };
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, this->timer_));
  ESP_ERROR_CHECK(esp_timer_start_periodic(*this->timer_, period));

  this->parent_->add_on_conversion_callback([this](float sample) { this->process_adc_conversion(sample); });

  ESP_LOGCONFIG(TAG, "AC Voltage sensor setup finished!");
}

void AcVoltageSensor::dump_config() {
  LOG_SENSOR("", "AC Voltage", this);

  ESP_LOGCONFIG(TAG, "  Multiplier: %.3f", this->multiplier_);
  ESP_LOGCONFIG(TAG, "  Midpoint: %.1fV", this->midpoint_);
  ESP_LOGCONFIG(TAG, "  Mains Frequency: %uHz", this->mains_frequency_);

  LOG_UPDATE_INTERVAL(this);
}

void AcVoltageSensor::update() {
  auto voltage = this->sample();
  ESP_LOGV(TAG, "'%s': Got RMS voltage=%.4fV", this->get_name().c_str(), voltage);
  this->publish_state(voltage);
}

float AcVoltageSensor::sample() { return this->voltage_rms_; }

void AcVoltageSensor::add_on_adc_conversion_callback(std::function<void(float)> &&callback) {
  this->adc_conversion_callback_.add(std::move(callback));
}

void AcVoltageSensor::add_on_period_callback(std::function<void(float)> &&callback) {
  this->period_callback_.add(std::move(callback));
}

void HOT AcVoltageSensor::process_adc_conversion(float sample) {
  auto voltage = abs(sample - this->midpoint_) * this->multiplier_;
  auto time = esp_timer_get_time();

  this->adc_conversion_callback_.call(voltage);

  taskENTER_CRITICAL(&this->voltage_sum_spinlock_);
  this->voltage_samples_squared_sum_ += voltage * voltage;
  this->voltage_samples_count_++;
  taskEXIT_CRITICAL(&this->voltage_sum_spinlock_);
}

void HOT AcVoltageSensor::measure_voltage() {
  taskENTER_CRITICAL(&this->voltage_sum_spinlock_);
  auto sum = this->voltage_samples_squared_sum_;
  auto count = this->voltage_samples_count_;
  this->voltage_samples_squared_sum_ = 0;
  this->voltage_samples_count_ = 0;
  taskEXIT_CRITICAL(&this->voltage_sum_spinlock_);

  this->voltage_rms_ = sqrtf(sum / count);

  this->period_callback_.call(this->voltage_rms_);

#ifdef ESPHOME_LOG_HAS_VERBOSE
  auto time = esp_timer_get_time();
  this->min_period_samples_ = std::min(this->min_period_samples_, count);
  this->max_period_samples_ = std::max(this->max_period_samples_, count);
  if (time % 1000000 < 20000) {
    auto min = this->min_period_samples_;
    auto max = this->max_period_samples_;
    auto voltage = this->voltage_rms_;
    this->defer([count, min, max, voltage]() {
      ESP_LOGV(TAG, "Count: %u, Min count: %u, Max count: %u, V RMS: %f", count, min, max, voltage);
    });
    this->min_period_samples_ = std::numeric_limits<uint32_t>::max();
    this->max_period_samples_ = std::numeric_limits<uint32_t>::min();
  }
#endif
}

void HOT AcVoltageSensor::timer_callback(void *arg) {
  AcVoltageSensor *ac_voltage = static_cast<AcVoltageSensor *>(arg);
  ac_voltage->measure_voltage();
}

}  // namespace ac_voltage
}  // namespace esphome