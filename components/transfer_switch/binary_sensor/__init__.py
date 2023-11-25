import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from .. import (
    transfer_switch_ns,
    TransferSwitchComponent,
    CONF_TRANSFER_SWITCH_ID,
)

TransferSwitchBinarySensor = transfer_switch_ns.class_(
    "TransferSwitchBinarySensor", binary_sensor.BinarySensor, cg.Component
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(TransferSwitchBinarySensor)
    .extend(
        {
            cv.GenerateID(CONF_TRANSFER_SWITCH_ID): cv.use_id(TransferSwitchComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_TRANSFER_SWITCH_ID])
