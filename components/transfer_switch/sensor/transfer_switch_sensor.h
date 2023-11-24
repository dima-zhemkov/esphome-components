#pragma once

#ifdef USE_ESP32

#include "../transfer_switch.h"

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"

namespace esphome {
namespace transfer_switch {

class TransferSwitchSensor : public sensor::Sensor,
                             public PollingComponent,
                             public voltage_sampler::VoltageSampler,
                             public Parented<TransferSwitchComponent> {
 public:
  void update() override;
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  float sample() override;

 private:
  float voltage_{0};
};

}  // namespace transfer_switch
}  // namespace esphome

#endif