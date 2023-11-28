#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fast_adc/fast_adc.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ac_voltage {

class AcVoltageSensor : public sensor::Sensor,
                        public PollingComponent,
                        public voltage_sampler::VoltageSampler,
                        public Parented<fast_adc::FastADCComponent> {
 public:
  void set_multiplier(float multiplier) { this->multiplier_ = multiplier; }
  void set_midpoint(float voltage) { this->midpoint_ = voltage; }
  void set_mains_frequency(uint32_t frequency) { this->mains_frequency_ = frequency; }

  void setup() override;
  void dump_config() override;

  void update() override;
  float sample() override;

  void add_on_adc_conversion_callback(std::function<void(float)> &&callback);
  void add_on_period_callback(std::function<void(float)> &&callback);

 protected:
  adc_channel_t adc_channel_{ADC_CHANNEL_MAX};
  float multiplier_{NAN};
  float midpoint_{NAN};
  uint32_t mains_frequency_{0};

  esp_timer_handle_t *timer_{nullptr};

  CallbackManager<void(float)> adc_conversion_callback_{};
  CallbackManager<void(float)> period_callback_{};

  portMUX_TYPE voltage_sum_spinlock_ = portMUX_INITIALIZER_UNLOCKED;

  float voltage_samples_squared_sum_{0};
  uint32_t voltage_samples_count_{0};
  float voltage_rms_{0};

#ifdef ESPHOME_LOG_HAS_VERBOSE
  uint32_t min_period_samples_{std::numeric_limits<uint32_t>::max()};
  uint32_t max_period_samples_{std::numeric_limits<uint32_t>::min()};
#endif

  void process_adc_conversion(float sample);
  void measure_voltage();

  static void timer_callback(void *arg);
};

}  // namespace ac_voltage
}  // namespace esphome