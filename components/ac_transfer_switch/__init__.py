import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, output
from esphome.components.ac_voltage import sensor as ac_voltage
from esphome.const import (
    CONF_ID,
    CONF_OUTPUT,
    CONF_POWER,
    DEVICE_CLASS_POWER,
)

CODEOWNERS = ["@dima-zhemkov"]

AUTO_LOAD = ["binary_sensor"]

CONF_TRANSFER_SWITCH_ID = "transfer_switch_id"
CONF_AC_VOLTAGE = "ac_voltage"
CONF_BACKUP_SWITCH_DELAY = "backup_switch_delay"
CONF_BACKUP_SWITCH_VOLTAGE_THRESHOLD = "backup_switch_voltage_threshold"
CONF_PRIMARY_SWITCH_DELAY = "primary_switch_delay"
CONF_PRIMARY_SWITCH_VOLTAGE_RMS_THRESHOLD = "primary_switch_voltage_rms_threshold"

ac_transfer_switch_ns = cg.esphome_ns.namespace("ac_transfer_switch")
AcTransferSwitchComponent = ac_transfer_switch_ns.class_("AcTransferSwitchComponent", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AcTransferSwitchComponent),
            cv.Required(CONF_AC_VOLTAGE): cv.use_id(ac_voltage.AcVoltageSensor),
            cv.Optional(CONF_BACKUP_SWITCH_DELAY, default="5ms"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_BACKUP_SWITCH_VOLTAGE_THRESHOLD, default="80V"): cv.voltage,
            cv.Optional(CONF_PRIMARY_SWITCH_DELAY, default="1s"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_PRIMARY_SWITCH_VOLTAGE_RMS_THRESHOLD, default="150V"): cv.voltage,
            cv.Optional(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
            cv.Optional(CONF_POWER): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_POWER,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_AC_VOLTAGE])

    cg.add(var.set_backup_switch_delay(config[CONF_BACKUP_SWITCH_DELAY]))
    cg.add(var.set_backup_switch_voltage_threshold(config[CONF_BACKUP_SWITCH_VOLTAGE_THRESHOLD]))
    cg.add(var.set_primary_switch_delay(config[CONF_PRIMARY_SWITCH_DELAY]))
    cg.add(var.set_primary_switch_voltage_rms_threshold(config[CONF_PRIMARY_SWITCH_VOLTAGE_RMS_THRESHOLD]))

    if CONF_OUTPUT in config:
        output_ = await cg.get_variable(config[CONF_OUTPUT])
        cg.add(var.set_output(output_))

    if CONF_POWER in config:
        sensor_ = await binary_sensor.new_binary_sensor(config[CONF_POWER])
        cg.add(var.set_power_sensor(sensor_))