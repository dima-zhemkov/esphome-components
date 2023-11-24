#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/components/fast_adc/fast_adc.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace transfer_switch {

class TransferSwitchComponent : public Component, public Parented<fast_adc::FastADCComponent> {
 public:
  void set_multiplier(float multiplier) { this->multiplier_ = multiplier; }
  void set_midpoint(float voltage) { this->midpoint_ = voltage; }
  void set_undervoltage_delay(uint32_t delay) { this->undervoltage_delay_ = delay; }
  void set_return_to_mains_delay(uint32_t delay) { this->return_to_mains_delay_ = delay; }
  void set_min_voltage_rms_threshold(float voltage) { this->min_voltage_rms_threshold_ = voltage; }
  void set_mains_frequency(uint32_t frequency) { this->mains_frequency_ = frequency; }
  void set_undervoltage_threshold(float voltage) { this->undervoltage_threshold_ = voltage; }

  void setup() override;
  void dump_config() override;

  void add_on_state_callback(std::function<void(bool)> &&callback);
  void add_on_rms_voltage_callback(std::function<void(float)> &&callback);

 private:
  adc_channel_t adc_channel_{ADC_CHANNEL_MAX};
  float multiplier_{NAN};
  float midpoint_{NAN};
  uint32_t undervoltage_delay_{0};     // delay in microseconds
  uint32_t return_to_mains_delay_{0};  // delay in microseconds
  float min_voltage_rms_threshold_{NAN};
  uint32_t mains_frequency_{0};
  float undervoltage_threshold_{NAN};

  CallbackManager<void(bool)> state_callback_{};
  Deduplicator<bool> state_dedup_;
  bool state_{false};

  esp_timer_handle_t *timer_{nullptr};
  int64_t undervoltage_start_{0};
  int64_t stable_voltage_start_{0};
  int64_t period_start_{0};

  CallbackManager<void(float)> rms_voltage_callback_{};
  Deduplicator<float> rms_voltage_dedup_;
  portMUX_TYPE voltage_sum_spinlock_ = portMUX_INITIALIZER_UNLOCKED;
  float voltage_samples_squared_sum_{0};
  uint32_t voltage_samples_count_{0};
  float voltage_rms_{0};

#ifdef ESPHOME_LOG_HAS_VERBOSE
  uint32_t min_period_samples{std::numeric_limits<uint32_t>::max()};
  uint32_t max_period_samples{std::numeric_limits<uint32_t>::min()};
#endif

  void set_state(bool state);

  void process_sample(float sample);
  void measure_voltage();

  int64_t frequency_to_microseconds(uint32_t frequency);

  static void timer_callback(void *arg);
};

}  // namespace transfer_switch
}  // namespace esphome

#endif
