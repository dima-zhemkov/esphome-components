#pragma once

#ifdef USE_ESP32

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include "driver/adc.h"
#include <esp_adc_cal.h>

namespace esphome {
namespace fast_adc {

class FastADCComponent : public Component {
 public:
  void set_pin(GPIOPin *pin) { this->pin_ = pin; }
  void set_channel(adc_channel_t channel) { this->channel_ = channel; }
  void set_frequency(uint32_t frequency) { this->frequency_ = frequency; }
  void set_oversampling(uint32_t oversampling) { this->oversampling_ = oversampling; }
  void set_calibration_data(const std::vector<float> &data) { this->calibration_data_ = data; }

  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void add_on_conversion_callback(std::function<void(float)> &&callback);

 protected:
  GPIOPin *pin_{nullptr};
  adc_channel_t channel_{ADC_CHANNEL_MAX};
  uint32_t frequency_{0};
  uint32_t oversampling_{0};
  std::vector<float> calibration_data_;

  CallbackManager<void(float)> conversion_callback_{};
  esp_adc_cal_characteristics_t adc_cal_characteristics_{};
  uint32_t buffer_max_length_{0};

  static void adc_task(void *pv);

  float adc_cal_raw_to_mv(uint16_t adc_reading);
};

}  // namespace fast_adc
}  // namespace esphome

#endif
