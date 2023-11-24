import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32, sensor, voltage_sampler
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
)
from .. import (
    transfer_switch_ns,
    TransferSwitchComponent,
    CONF_TRANSFER_SWITCH_ID,
)

AUTO_LOAD = ["voltage_sampler"]


TransferSwitchSensor = transfer_switch_ns.class_(
    "TransferSwitchSensor", sensor.Sensor, cg.PollingComponent, voltage_sampler.VoltageSampler
)

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        TransferSwitchSensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_TRANSFER_SWITCH_ID): cv.use_id(TransferSwitchComponent),
        }
    )
    .extend(cv.polling_component_schema("60s")),
    cv.only_on_esp32,
    esp32.only_on_variant(supported=[esp32.const.VARIANT_ESP32]),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)


    parent = await cg.get_variable(config[CONF_TRANSFER_SWITCH_ID])
    cg.add(var.set_parent(parent))


