import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fast_adc, sensor, voltage_sampler
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
)

CODEOWNERS = ["@dima-zhemkov"]

DEPENDENCIES = ["fast_adc"]

AUTO_LOAD = ["voltage_sampler"]

MAINS_FREQUENCIES = {
    "50HZ": 50,
    "60HZ": 60,
}

CONF_FAST_ADC_ID = "fast_adc_id"
CONF_MULTIPLIER = "multiplier"
CONF_MIDPIONT = "midpoint"
CONF_MAINS_FREQUENCY = "mains_frequency"

ac_voltage_ns = cg.esphome_ns.namespace("ac_voltage")
AcVoltageSensor = ac_voltage_ns.class_(
    "AcVoltageSensor", sensor.Sensor, cg.PollingComponent, voltage_sampler.VoltageSampler
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
            cv.GenerateID(CONF_FAST_ADC_ID): cv.use_id(fast_adc.FastADCComponent),
            cv.Optional(CONF_MIDPIONT, default="1.5V"): cv.voltage,
            cv.Optional(CONF_MULTIPLIER, default=207.4): cv.float_range(min=0, min_included=False),
            cv.Optional(CONF_MAINS_FREQUENCY, default="50HZ"): cv.enum(MAINS_FREQUENCIES, upper=True),
        }
    )
    .extend(cv.polling_component_schema("60s")),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
    await cg.register_parented(var, config[CONF_FAST_ADC_ID])

    cg.add(var.set_midpoint(config[CONF_MIDPIONT]))
    cg.add(var.set_multiplier(config[CONF_MULTIPLIER]))
    cg.add(var.set_mains_frequency(config[CONF_MAINS_FREQUENCY]))