import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fast_adc, sensor, voltage_sampler
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
)
from . import (
    AcVoltageSensor,
    MAINS_FREQUENCIES,
    CONF_FAST_ADC,
    CONF_MULTIPLIER,
    CONF_MIDPIONT,
    CONF_MAINS_FREQUENCY,
)

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        AcVoltageSensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.GenerateID(CONF_FAST_ADC): cv.use_id(fast_adc.FastADCComponent),
            cv.Optional(CONF_MULTIPLIER, default=207.4): cv.float_range(min=0, min_included=False),
            cv.Optional(CONF_MIDPIONT, default="311V"): cv.voltage,
            cv.Optional(CONF_MAINS_FREQUENCY, default="50HZ"): cv.enum(MAINS_FREQUENCIES, upper=True),
        }
    )
    .extend(cv.polling_component_schema("60s")),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_FAST_ADC])
    cg.add(var.set_parent(parent))

    cg.add(var.set_multiplier(config[CONF_MULTIPLIER]))
    cg.add(var.set_midpoint(config[CONF_MIDPIONT]))
    cg.add(var.set_mains_frequency(config[CONF_MAINS_FREQUENCY]))