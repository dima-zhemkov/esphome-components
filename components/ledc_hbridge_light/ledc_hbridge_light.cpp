#include "ledc_hbridge_light.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ledc_hbridge_light {

static const char *const TAG = "ledc_hbridge_light";
static const float step = 5000.0f;

light::LightTraits LedcHbridgeLightOutput::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
  traits.set_min_mireds(this->cold_white_temperature_);
  traits.set_max_mireds(this->warm_white_temperature_);
  return traits;
}

void LedcHbridgeLightOutput::write_state(light::LightState *state) {
  float cwhite, wwhite;
  state->current_values_as_cwww(&cwhite, &wwhite, this->constant_brightness_);

  float frequency_cwhite = this->calculate_frequency(cwhite, this->cold_white_->get_max_power(), this->cold_white_->get_min_power());
  float frequency_wwhite = this->calculate_frequency(wwhite, this->warm_white_->get_max_power(), this->warm_white_->get_min_power());
  float frequency = frequency_cwhite > frequency_wwhite ? frequency_cwhite : frequency_wwhite;
  cwhite = this->adjust_state(cwhite, frequency, this->cold_white_->get_max_power(), this->cold_white_->get_min_power());
  wwhite = this->adjust_state(wwhite, frequency, this->warm_white_->get_max_power(), this->warm_white_->get_min_power());

  if (frequency != this->frequency_)
  {
    ESP_LOGD(TAG, "Frequency changed: %.0f Hz -> %.0f Hz", this->frequency_, frequency);
    this->cold_white_->update_frequency(frequency);
    this->warm_white_->update_frequency(frequency);
    this->frequency_ = frequency;
  }

  this->cold_white_->set_level(cwhite);
  this->warm_white_->set_level(wwhite);
}

float LedcHbridgeLightOutput::calculate_frequency(float state, float max_power, float min_power) {
  float real_duty_cycle = get_real_duty_cycle(state, max_power, min_power);
  
  float max_period = 1.0f / this->max_frequency_;
  float min_duty_cycle = this->min_pulse_ / max_period;
  
  float frequency;
  if (real_duty_cycle >= min_duty_cycle || this->min_frequency_ == 0.0f) {
    float required_period = this->min_pulse_ / real_duty_cycle;
    frequency = 1.0f / required_period;
    
    if (frequency < this->min_frequency_)
      frequency = this->min_frequency_;
    else if (frequency > this->max_frequency_)
      frequency = this->max_frequency_;
    
    frequency = round(frequency);
  } else {
    frequency = this->min_frequency_;
  }
  
  return frequency;
}

float LedcHbridgeLightOutput::adjust_state(float state, float frequency, float max_power, float min_power) {
  float real_duty_cycle = get_real_duty_cycle(state, max_power, min_power);
  
  float max_period = 1.0f / this->max_frequency_;
  float min_duty_cycle = this->min_pulse_ / max_period;
  
  float corrected_duty_cycle;
  if (frequency == this->max_frequency_) {
    corrected_duty_cycle = state;
  } else {
    corrected_duty_cycle = state * this->low_freq_brightness_correction_;
    
    float corrected_real_duty_cycle = get_real_duty_cycle(corrected_duty_cycle, max_power, min_power);
    float period = 1.0f / frequency;
    float min_duty_cycle_for_freq = this->min_pulse_ / period;
    
    if (corrected_real_duty_cycle > 0.0f && corrected_real_duty_cycle < min_duty_cycle_for_freq) {
      corrected_duty_cycle = get_unscaled_duty_cycle(min_duty_cycle_for_freq, max_power, min_power);
    }
  }
  
  return corrected_duty_cycle;
}

float LedcHbridgeLightOutput::get_unscaled_duty_cycle(float state, float max_power, float min_power) {
  if (max_power > min_power) {
    return (state - min_power) / (max_power - min_power);
  } else {
    return state;
  }
}

float LedcHbridgeLightOutput::get_real_duty_cycle(float state, float max_power, float min_power) {
  if (max_power > min_power) {
    return state * (max_power - min_power) + min_power;
  } else {
    return state;
  }
}

}  // namespace ledc_hbridge_light
}  // namespace esphome
