#include "ledc_hbridge_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ledc_hbridge_light {

static const char *const TAG = "ledc_hbridge_light";

float LedcHbridgeLightOutput::calculate_frequency(float state) {
  float input = state;
  float frequency;

  float T_max = 1.0f / this->max_frequency_;
  float D_min = this->min_pulse_ / T_max;
  
  ESP_LOGD(TAG, "calc_freq: state=%.4f, T_max=%.6f, min_pulse=%.9f, D_min=%.4f", 
           state, T_max, this->min_pulse_, D_min);

  if (input >= D_min) {
    frequency = this->max_frequency_;
  } else if (input > 0.0f) {
    float T = this->min_pulse_ / input;
    frequency = 1.0f / T;
    
    ESP_LOGD(TAG, "calc_freq: T=%.6f, freq_before_limit=%.2f", T, frequency);

    if (frequency < this->min_frequency_)
      frequency = this->min_frequency_;
    else if (frequency > this->max_frequency_)
      frequency = this->max_frequency_;
  } else {
    frequency = this->max_frequency_;
  }
  
  ESP_LOGD(TAG, "calc_freq: final frequency=%.2f", frequency);
  return frequency;
}

float LedcHbridgeLightOutput::adjust_state(float state, float frequency) {
  float input = state;
  float adjusted_state;

  float T_max = 1.0f / this->max_frequency_;
  float D_min = this->min_pulse_ / T_max;

  if (input >= D_min) {
    adjusted_state = input;
  } else if (input > 0.0f) {
    adjusted_state = this->min_pulse_ * frequency;
    ESP_LOGD(TAG, "adjust_state: input=%.4f < D_min=%.4f, adjusted=%.4f (min_pulse=%.6f * freq=%.2f)", 
             input, D_min, adjusted_state, this->min_pulse_, frequency);
  } else {
    adjusted_state = 0.0f;
  }
  
  return adjusted_state;
}

}  // namespace ledc_hbridge_light
}  // namespace esphome
