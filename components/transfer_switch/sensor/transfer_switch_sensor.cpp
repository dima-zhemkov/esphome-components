#ifdef USE_ESP32

#include "transfer_switch_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace transfer_switch {

static const char *const TAG = "transfer_switch.sensor";

void TransferSwitchSensor::setup() {
  parent_->add_on_rms_voltage_callback([this](float voltage) { this->voltage_ = voltage; });
}

void TransferSwitchSensor::dump_config() {
  LOG_SENSOR("", "Transfer Switch Sensor", this);

  LOG_UPDATE_INTERVAL(this);
}

float TransferSwitchSensor::get_setup_priority() const { return setup_priority::DATA; }

void TransferSwitchSensor::update() {
  auto voltage = sample();
  ESP_LOGV(TAG, "'%s': Got voltage=%.4fV", get_name().c_str(), voltage);
  publish_state(voltage);
}

float TransferSwitchSensor::sample() { return voltage_; }

}  // namespace transfer_switch
}  // namespace esphome

#endif