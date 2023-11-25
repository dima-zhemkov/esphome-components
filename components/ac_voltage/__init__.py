import esphome.codegen as cg
from esphome.components import sensor, voltage_sampler

CODEOWNERS = ["@dima-zhemkov"]

DEPENDENCIES = ["fast_adc"]

AUTO_LOAD = ["voltage_sampler"]

MAINS_FREQUENCIES = {
    "50HZ": 50,
    "60HZ": 60,
}

CONF_FAST_ADC = "fast_adc_id"
CONF_MULTIPLIER = "multiplier"
CONF_MIDPIONT = "midpoint"
CONF_MAINS_FREQUENCY = "mains_frequency"

ac_voltage_ns = cg.esphome_ns.namespace("ac_voltage")
AcVoltageSensor = ac_voltage_ns.class_(
    "AcVoltageSensor", sensor.Sensor, cg.PollingComponent, voltage_sampler.VoltageSampler
)