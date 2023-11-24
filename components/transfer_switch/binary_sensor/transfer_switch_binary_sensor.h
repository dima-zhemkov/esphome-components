#pragma once

#ifdef USE_ESP32

#include "../transfer_switch.h"

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace transfer_switch {

class TransferSwitchBinarySensor : public Component,
                                   public binary_sensor::BinarySensor,
                                   public Parented<TransferSwitchComponent> {
 public:
  void setup() override;
  void dump_config() override;
  void loop() override;

 protected:
  float get_setup_priority() const override;

 private:
  bool state_{false};
};

}  // namespace transfer_switch
}  // namespace esphome

#endif