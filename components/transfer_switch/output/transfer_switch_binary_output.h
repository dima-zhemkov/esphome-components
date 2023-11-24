#pragma once

#ifdef USE_ESP32

#include "../transfer_switch.h"

#include "esphome/core/component.h"
#include "esphome/components/output/binary_output.h"

namespace esphome {
namespace transfer_switch {

class TransferSwitchBinaryOutput : public Component,
                                   public output::BinaryOutput,
                                   public Parented<TransferSwitchComponent> {
 public:
  void set_pin(GPIOPin *pin) { this->pin_ = pin; }

  void setup() override;
  void dump_config() override;

 protected:
  void write_state(bool state) override;

 private:
  GPIOPin *pin_{nullptr};
};

}  // namespace transfer_switch
}  // namespace esphome

#endif
