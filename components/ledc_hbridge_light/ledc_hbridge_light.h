#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ledc/ledc_output.h"
#include "esphome/components/light/light_output.h"

namespace esphome {
namespace ledc_hbridge_light {

class LedcHbridgeLightOutput : public light::LightOutput {
 public:
  void set_cold_white(ledc::LEDCOutput *cold_white) { cold_white_ = cold_white; }
  void set_warm_white(ledc::LEDCOutput *warm_white) { warm_white_ = warm_white; }
  void set_cold_white_temperature(float cold_white_temperature) { cold_white_temperature_ = cold_white_temperature; }
  void set_warm_white_temperature(float warm_white_temperature) { warm_white_temperature_ = warm_white_temperature; }
  void set_constant_brightness(bool constant_brightness) { constant_brightness_ = constant_brightness; }
  void set_max_frequency(float max_frequency) { max_frequency_ = max_frequency; }
  void set_min_frequency(float min_frequency) { min_frequency_ = min_frequency; }
  void set_min_pulse(float min_pulse) { min_pulse_ = min_pulse; }
  light::LightTraits get_traits() override;
  void write_state(light::LightState *state) override;

 protected:
  ledc::LEDCOutput *cold_white_;
  ledc::LEDCOutput *warm_white_;
  float cold_white_temperature_{0};
  float warm_white_temperature_{0};
  bool constant_brightness_{false};
  float max_frequency_{0};
  float min_frequency_{0};
  float min_pulse_{0};
  float frequency_{0};
  
  float calculate_frequency(float state, float max_power, float min_power);
  float adjust_state(float state, float frequency, float max_power, float min_power);
  float get_unscaled_duty_cycle(float state, float max_power, float min_power);
  float get_real_duty_cycle(float state, float max_power, float min_power);
};

}  // namespace ledc_hbridge_light
}  // namespace esphome