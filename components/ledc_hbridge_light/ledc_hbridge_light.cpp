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
  
  if (input >= D_min) {
    frequency = this->max_frequency_;
  } else if (input > 0.0f) {
    float T = this->min_pulse_ / input;
    frequency = 1.0f / T;
    
    if (frequency < this->min_frequency_)
      frequency = this->min_frequency_;
    else if (frequency > this->max_frequency_)
      frequency = this->max_frequency_;
  } else {
    frequency = this->max_frequency_;
  }
  
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
    float expected_pulse_ns = adjusted_state * (1.0f / frequency) * 1e9;
  } else {
    adjusted_state = 0.0f;
  }
  
  return adjusted_state;
}

}  // namespace ledc_hbridge_light
}  // namespace esphome
