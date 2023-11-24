#include "yeelight_ceiling.h"
#include "esphome/core/application.h"

namespace esphome {
namespace yeelight_ceiling {

light::LightTraits YeelightCeiling::get_traits() {
  auto traits = light::LightTraits();
  traits.set_supported_color_modes({light::ColorMode::COLD_WARM_WHITE});
  traits.set_min_mireds(cold_white_temperature_);
  traits.set_max_mireds(warm_white_temperature_);
  return traits;
}

void YeelightCeiling::write_state(light::LightState *state) {
  float brightness, cwhite, wwhite;
  state->current_values_as_brightness(&brightness);
  if (brightness > night_light_brightness_threshold_) {
    state->current_values_as_cwww(&cwhite, &wwhite, constant_brightness_);
    cold_white_output_->set_level(cwhite);
    warm_white_output_->set_level(wwhite);
    if (is_night_light_turn_on) {
      night_light_output_->set_level(1.0f);
      is_night_light_turn_on = false;
      set_timeout(state, "turn_off_night_light", night_light_turn_off_delay_,
                  [this]() { this->night_light_output_->set_level(0.0f); });
    }
  } else {
    float night_light_brightness = brightness / night_light_brightness_threshold_;
    cold_white_output_->turn_off();
    warm_white_output_->turn_off();
    cancel_timeout(state, "turn_off_night_light");
    night_light_set_level(night_light_brightness);
    is_night_light_turn_on = night_light_brightness > 0.0f;
  }
}

void YeelightCeiling::set_timeout(light::LightState *state, const std::string &name, uint32_t timeout,
                                  std::function<void()> &&f) {
  return App.scheduler.set_timeout(state, name, timeout, std::move(f));
}

bool YeelightCeiling::cancel_timeout(light::LightState *state, const std::string &name) {
  return App.scheduler.cancel_timeout(state, name);
}

}  // namespace yeelight_ceiling
}  // namespace esphome