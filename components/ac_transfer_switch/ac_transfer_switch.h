#pragma once

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/ac_voltage/ac_voltage.h"

namespace esphome {
namespace ac_transfer_switch {

class AcTransferSwitchComponent : public Component, public Parented<ac_voltage::AcVoltageSensor> {
 public:
  void set_output(output::BinaryOutput *output) { this->output_ = output; }
  void set_power_sensor(binary_sensor::BinarySensor *sensor) { this->power_sensor_ = sensor; }
  void set_backup_switch_delay(uint32_t delay) { this->backup_switch_delay_ = delay; }
  void set_backup_switch_voltage_threshold(float voltage) { this->backup_switch_voltage_threshold_ = voltage; }
  void set_primary_switch_delay(uint32_t delay) { this->primary_switch_delay_ = delay; }
  void set_primary_switch_voltage_rms_threshold(float voltage) {
    this->primary_switch_voltage_rms_threshold_ = voltage;
  }

  void setup() override;
  void dump_config() override;

 protected:
  output::BinaryOutput *output_{nullptr};
  binary_sensor::BinarySensor *power_sensor_{nullptr};
  uint32_t backup_switch_delay_{0};  // delay in microseconds
  float backup_switch_voltage_threshold_{NAN};
  uint32_t primary_switch_delay_{0};  // delay in microseconds
  float primary_switch_voltage_rms_threshold_{NAN};

  enum class PowerSource : uint32_t {
    UNKNOWN = 0,
    PRIMARY = 1,
    BACKUP = 2,
  };

  Deduplicator<PowerSource> power_source_dedup_;
  PowerSource power_source_{PowerSource::PRIMARY};

  esp_timer_handle_t *timer_{nullptr};
  int64_t undervoltage_start_{0};
  int64_t stable_voltage_start_{0};

  void set_power_source(PowerSource value);

  void process_adc_conversion(float voltage);
  void process_period(float voltage_rms);
};

}  // namespace ac_transfer_switch
}  // namespace esphome