import math
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import esp32, fast_adc
from esphome.const import (
    CONF_ID,
    CONF_OVERSAMPLING,
)

CODEOWNERS = ["@dima-zhemkov"]

DEPENDENCIES = ["fast_adc"]

MAINS_FREQUENCIES = {
    "50HZ": 50,
    "60HZ": 60,
}

CONF_FAST_ADC = "fast_adc_id"
CONF_MULTIPLIER = "multiplier"
CONF_MIDPIONT = "midpoint"
CONF_UNDERVOLTAGE_DELAY = "undervoltage_delay"
CONF_RETURN_TO_MAINS_DELAY = "return_to_mains_delay"
CONF_MIN_VOLTAGE_RMS_THRESHOLD = "min_voltage_rms"
CONF_MAINS_FREQUENCY = "mains_frequency"
CONF_TRANSFER_SWITCH_ID = "transfer_switch_id"

transfer_switch_ns = cg.esphome_ns.namespace("transfer_switch")
TransferSwitchComponent = transfer_switch_ns.class_("TransferSwitchComponent", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(TransferSwitchComponent),
            cv.GenerateID(CONF_FAST_ADC): cv.use_id(fast_adc.FastADCComponent),
            cv.Optional(CONF_MULTIPLIER, default=207.4): cv.float_range(min=0, min_included=False),
            cv.Optional(CONF_MIDPIONT, default=311.0): cv.voltage,
            cv.Optional(CONF_UNDERVOLTAGE_DELAY, default="5ms"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_RETURN_TO_MAINS_DELAY, default="1s"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_MIN_VOLTAGE_RMS_THRESHOLD, default=150.0): cv.voltage,
            cv.Optional(CONF_MAINS_FREQUENCY, default="50HZ"): cv.enum(MAINS_FREQUENCIES, upper=True),
            cv.Optional(CONF_OVERSAMPLING, default=40): cv.int_range(min=40, max=100)
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    cv.only_on_esp32,
    esp32.only_on_variant(supported=[esp32.const.VARIANT_ESP32])
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    parent = await cg.get_variable(config[CONF_FAST_ADC])
    cg.add(var.set_parent(parent))

    cg.add(var.set_multiplier(config[CONF_MULTIPLIER]))
    cg.add(var.set_midpoint(config[CONF_MIDPIONT]))

    undervoltage_delay = config.get(CONF_UNDERVOLTAGE_DELAY)
    cg.add(var.set_undervoltage_delay(undervoltage_delay))
    cg.add(var.set_return_to_mains_delay(config[CONF_RETURN_TO_MAINS_DELAY]))

    min_voltage_rms_threshold = config.get(CONF_MIN_VOLTAGE_RMS_THRESHOLD)
    cg.add(var.set_min_voltage_rms_threshold(min_voltage_rms_threshold))

    mains_frequency = config.get(CONF_MAINS_FREQUENCY)
    cg.add(var.set_mains_frequency(mains_frequency))

    oversampling = config.get(CONF_OVERSAMPLING)

    period_duration = 1000000.0 / MAINS_FREQUENCIES[mains_frequency]
    sample_duration = period_duration / oversampling
    undervoltage_delay_radians = math.pi * (undervoltage_delay.total_microseconds % period_duration) / period_duration
    sample_duration_radians = math.pi * 2 * sample_duration / period_duration
    min_voltage_threshold = min_voltage_rms_threshold * math.sqrt(2)
    undervoltage_threshold_error = min_voltage_threshold * math.sin(sample_duration_radians)
    undervoltage_threshold = min_voltage_threshold * math.sin(undervoltage_delay_radians)
    undervoltage_threshold_including_error = max(undervoltage_threshold - undervoltage_threshold_error, 0)
    cg.add(var.set_undervoltage_threshold(undervoltage_threshold_including_error))