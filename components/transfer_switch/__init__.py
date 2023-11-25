import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

CODEOWNERS = ["@dima-zhemkov"]

CONF_AC_VOLTAGE_ID = "ac_voltage_id"
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
            cv.Required(CONF_AC_VOLTAGE_ID): cv.use_id(sensor.Sensor),
            cv.Optional(CONF_INSTANT_SWITCH_DELAY, default="5ms"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_RETURN_TO_MAINS_DELAY, default="1s"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_MIN_VOLTAGE_RMS_THRESHOLD, default="150V"): cv.voltage,
            cv.Optional(CONF_INSTANT_SWITCH_VOLTAGE_THRESHOLD, default="80V"): cv.voltage,
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    sensor = await cg.get_variable(config[CONF_AC_VOLTAGE_ID])
    cg.add(var.set_sensor(sensor))

    cg.add(var.set_instant_switch_delay(config[CONF_INSTANT_SWITCH_DELAY]))
    cg.add(var.set_return_to_mains_delay(config[CONF_RETURN_TO_MAINS_DELAY]))
    cg.add(var.set_min_voltage_rms_threshold(config[CONF_MIN_VOLTAGE_RMS_THRESHOLD]))
    cg.add(var.set_instant_switch_voltage_threshold(config[CONF_INSTANT_SWITCH_VOLTAGE_THRESHOLD]))