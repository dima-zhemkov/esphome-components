#include "transfer_switch_binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace transfer_switch {

static const char *const TAG = "transfer_switch.binary_sensor";

void TransferSwitchBinarySensor::setup() {
  parent_->add_on_state_callback([this](bool state) { this->state_ = state; });
}

void TransferSwitchBinarySensor::dump_config() { LOG_BINARY_SENSOR("", "Transfer Switch Binary Sensor", this); }

float TransferSwitchBinarySensor::get_setup_priority() const { return setup_priority::HARDWARE; }

void TransferSwitchBinarySensor::loop() { publish_state(this->state_); }

}  // namespace transfer_switch
}  // namespace esphome