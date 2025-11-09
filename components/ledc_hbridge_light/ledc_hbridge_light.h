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
  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
    traits.set_min_mireds(this->cold_white_temperature_);
    traits.set_max_mireds(this->warm_white_temperature_);
    return traits;
  }
  void write_state(light::LightState *state) override {
    float cwhite, wwhite;
    state->current_values_as_cwww(&cwhite, &wwhite, this->constant_brightness_);
    this->cold_white_->set_level(cwhite);

    float phase_angle = cwhite * 360.0f;
    float adjusted_wwhite = wwhite * (1.0f - cwhite) * (4095.0f / 4096.0f);
    this->warm_white_->set_phase_angle(phase_angle);
    this->warm_white_->set_level(adjusted_wwhite);
  }

 protected:
  ledc::LEDCOutput *cold_white_;
  ledc::LEDCOutput *warm_white_;
  float cold_white_temperature_{0};
  float warm_white_temperature_{0};
  bool constant_brightness_{false};
};

}  // namespace ledc_hbridge_light
}  // namespace esphome