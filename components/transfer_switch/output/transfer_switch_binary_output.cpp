#include "transfer_switch_binary_output.h"
#include "esphome/core/log.h"

namespace esphome {
namespace transfer_switch {

static const char *const TAG = "transfer_switch.output";

void TransferSwitchBinaryOutput::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Transfer Switch Binary Output...");

  pin_->setup();
  turn_off();

  parent_->add_on_state_callback([this](bool state) { this->set_state(state); });

  ESP_LOGCONFIG(TAG, "Transfer Switch Binary Output setup finished!");
}

void TransferSwitchBinaryOutput::dump_config() {
  ESP_LOGCONFIG(TAG, "Transfer Switch Binary Output:");
  LOG_PIN("  Pin: ", pin_);
  LOG_BINARY_OUTPUT(this);
}

void TransferSwitchBinaryOutput::write_state(bool state) { pin_->digital_write(state); }

}  // namespace transfer_switch
}  // namespace esphome