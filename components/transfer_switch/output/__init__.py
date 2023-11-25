import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import output
from esphome.const import (
    CONF_ID,
    CONF_PIN,
)
from .. import (
    transfer_switch_ns,
    TransferSwitchComponent,
    CONF_TRANSFER_SWITCH_ID,
)

TransferSwitchBinaryOutput = transfer_switch_ns.class_(
    "TransferSwitchBinaryOutput", output.BinaryOutput, cg.Component
)

CONFIG_SCHEMA = cv.All(
    output.BINARY_OUTPUT_SCHEMA
    .extend(
        {
            cv.Required(CONF_ID): cv.declare_id(TransferSwitchBinaryOutput),
            cv.GenerateID(CONF_TRANSFER_SWITCH_ID): cv.use_id(TransferSwitchComponent),
            cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await output.register_output(var, config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_TRANSFER_SWITCH_ID])

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))