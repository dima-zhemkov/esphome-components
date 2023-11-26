import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, output
from esphome.components.ac_voltage import sensor as ac_voltage
from esphome.const import (
    CONF_ID,
    CONF_OUTPUT,
    CONF_STATE,
    DEVICE_CLASS_SWITCH,
)

CODEOWNERS = ["@dima-zhemkov"]

AUTO_LOAD = ["binary_sensor"]

CONF_AC_VOLTAGE = "ac_voltage"
CONF_INSTANT_SWITCH_DELAY = "instant_switch_delay"
CONF_RETURN_TO_MAINS_DELAY = "return_to_mains_delay"
CONF_MIN_VOLTAGE_RMS_THRESHOLD = "min_voltage_rms"
CONF_TRANSFER_SWITCH_ID = "transfer_switch_id"
CONF_INSTANT_SWITCH_VOLTAGE_THRESHOLD = "instant_switch_voltage_threshold"

transfer_switch_ns = cg.esphome_ns.namespace("transfer_switch")
TransferSwitchComponent = transfer_switch_ns.class_("TransferSwitchComponent", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TransferSwitchComponent),
            cv.Required(CONF_AC_VOLTAGE): cv.use_id(ac_voltage.AcVoltageSensor),
            cv.Required(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
            cv.Optional(CONF_INSTANT_SWITCH_DELAY, default="5ms"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_RETURN_TO_MAINS_DELAY, default="1s"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_MIN_VOLTAGE_RMS_THRESHOLD, default="150V"): cv.voltage,
            cv.Optional(CONF_INSTANT_SWITCH_VOLTAGE_THRESHOLD, default="80V"): cv.voltage,
            cv.Optional(CONF_STATE): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_SWITCH,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_AC_VOLTAGE])

    cg.add(var.set_instant_switch_delay(config[CONF_INSTANT_SWITCH_DELAY]))
    cg.add(var.set_return_to_mains_delay(config[CONF_RETURN_TO_MAINS_DELAY]))
    cg.add(var.set_min_voltage_rms_threshold(config[CONF_MIN_VOLTAGE_RMS_THRESHOLD]))
    cg.add(var.set_instant_switch_voltage_threshold(config[CONF_INSTANT_SWITCH_VOLTAGE_THRESHOLD]))

    output_ = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(output_))

    if CONF_STATE in config:
        sensor_ = await binary_sensor.new_binary_sensor(config[CONF_STATE])
        cg.add(var.set_state_sensor(sensor_))