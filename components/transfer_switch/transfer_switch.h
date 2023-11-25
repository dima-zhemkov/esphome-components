#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/ac_voltage/ac_voltage.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace transfer_switch {

class TransferSwitchComponent : public Component {
 public:
  void set_sensor(sensor::Sensor *sensor) { ac_sensor_ = dynamic_cast<ac_voltage::AcVoltageSensor *>(sensor); }
  void set_instant_switch_delay(uint32_t delay) { instant_switch_delay_ = delay; }
  void set_return_to_mains_delay(uint32_t delay) { return_to_mains_delay_ = delay; }
  void set_min_voltage_rms_threshold(float voltage) { min_voltage_rms_threshold_ = voltage; }
  void set_instant_switch_voltage_threshold(float voltage) { instant_switch_voltage_threshold_ = voltage; }

  void setup() override;
  void dump_config() override;

  void add_on_state_callback(std::function<void(bool)> &&callback);

 private:
  ac_voltage::AcVoltageSensor *ac_sensor_;
  uint32_t instant_switch_delay_{0};   // delay in microseconds
  uint32_t return_to_mains_delay_{0};  // delay in microseconds
  float min_voltage_rms_threshold_{NAN};
  float instant_switch_voltage_threshold_{NAN};

  CallbackManager<void(bool)> state_callback_{};
  Deduplicator<bool> state_dedup_;
  bool state_{false};

  esp_timer_handle_t *timer_{nullptr};
  int64_t undervoltage_start_{0};
  int64_t stable_voltage_start_{0};

  void set_state(bool state);

  void process_adc_conversion(float voltage);
  void process_period(float voltage_rms);
};

}  // namespace transfer_switch
}  // namespace esphome