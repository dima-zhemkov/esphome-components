#include "ledc_hbridge_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ledc_hbridge_light {

static const char *const TAG = "ledc_hbridge_light";

float LedcHbridgeLightOutput::calculate_frequency(float state, float max_power, float min_power) {
  float unscaled_duty_cycle;
  if (max_power > min_power && state > min_power) {
    unscaled_duty_cycle = (state - min_power) / (max_power - min_power);
  } else {
    unscaled_duty_cycle = state;
  }
  
  float max_period = 1.0f / this->max_frequency_;
  float min_duty_cycle = this->min_pulse_ / max_period;
  
  float frequency;
  if (unscaled_duty_cycle >= min_duty_cycle) {
    frequency = this->max_frequency_;
  } else if (unscaled_duty_cycle > 0.0f) {
    float required_period = this->min_pulse_ / unscaled_duty_cycle;
    frequency = 1.0f / required_period;
    
    if (frequency < this->min_frequency_)
      frequency = this->min_frequency_;
    else if (frequency > this->max_frequency_)
      frequency = this->max_frequency_;
  } else {
    frequency = this->max_frequency_;
  }
  
  return frequency;
}

float LedcHbridgeLightOutput::adjust_state(float state, float frequency, float max_power, float min_power) {
  float unscaled_duty_cycle;
  if (max_power > min_power && state > min_power) {
    unscaled_duty_cycle = (state - min_power) / (max_power - min_power);
  } else {
    unscaled_duty_cycle = state;
  }
  
  float max_period = 1.0f / this->max_frequency_;
  float min_duty_cycle = this->min_pulse_ / max_period;
  
  float corrected_duty_cycle;
  if (unscaled_duty_cycle >= min_duty_cycle) {
    corrected_duty_cycle = unscaled_duty_cycle;
  } else if (unscaled_duty_cycle > 0.0f) {
    corrected_duty_cycle = this->min_pulse_ * frequency;
  } else {
    corrected_duty_cycle = 0.0f;
  }
  
  float scaled_output;
  if (max_power > min_power) {
    scaled_output = (corrected_duty_cycle - min_power) / (max_power - min_power);
  } else {
    scaled_output = corrected_duty_cycle;
  }
  
  return scaled_output;
}

}  // namespace ledc_hbridge_light
}  // namespace esphome
