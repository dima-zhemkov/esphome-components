#include "esphome/core/component.h"
#include "esphome/components/light/light_output.h"
#include "esphome/components/output/float_output.h"

namespace esphome {
namespace yeelight_ceiling {

class YeelightCeiling : public light::LightOutput {
 public:
  void set_cold_white(output::FloatOutput *cold_white) { cold_white_output_ = cold_white; }
  void set_warm_white(output::FloatOutput *warm_white) { warm_white_output_ = warm_white; }
  void set_night_light(output::FloatOutput *night_light) { night_light_output_ = night_light; }
  void set_cold_white_temperature(float cold_white_temperature) { cold_white_temperature_ = cold_white_temperature; }
  void set_warm_white_temperature(float warm_white_temperature) { warm_white_temperature_ = warm_white_temperature; }
  void set_constant_brightness(bool constant_brightness) { constant_brightness_ = constant_brightness; }
  void set_night_light_brightness_threshold(float brightness) { night_light_brightness_threshold_ = brightness; }
  void set_night_light_turn_off_delay(uint32_t delay) { night_light_turn_off_delay_ = delay; }

  light::LightTraits get_traits() override;

  void write_state(light::LightState *state) override;

 private:
  output::FloatOutput *cold_white_output_{nullptr};
  output::FloatOutput *warm_white_output_{nullptr};
  output::FloatOutput *night_light_output_{nullptr};
  float cold_white_temperature_{NAN};
  float warm_white_temperature_{NAN};
  bool constant_brightness_{false};
  float night_light_brightness_threshold_{NAN};
  uint32_t night_light_turn_off_delay_{0};

  bool is_night_light_turn_on{false};

  void set_timeout(light::LightState *state, const std::string &name, uint32_t timeout, std::function<void()> &&f);
  bool cancel_timeout(light::LightState *state, const std::string &name);
};

}  // namespace yeelight_ceiling
}  // namespace esphome