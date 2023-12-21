import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import (
    CONF_ID,
    CONF_PIN,
    CONF_NUMBER,
    CONF_FREQUENCY,
    CONF_CALIBRATION,
)
from esphome.core import CORE
from esphome.components import esp32
from esphome.components.esp32 import get_esp32_variant
from esphome.components.esp32.const import (
    VARIANT_ESP32,
)

CODEOWNERS = ["@dima-zhemkov"]

adc_channel_t = cg.global_ns.enum("adc_channel_t")

# pin to adc1 channel mapping
ADC_PIN_TO_CHANNEL = {
    36: adc_channel_t.ADC_CHANNEL_0,
    37: adc_channel_t.ADC_CHANNEL_1,
    38: adc_channel_t.ADC_CHANNEL_2,
    39: adc_channel_t.ADC_CHANNEL_3,
    32: adc_channel_t.ADC_CHANNEL_4,
    33: adc_channel_t.ADC_CHANNEL_5,
    34: adc_channel_t.ADC_CHANNEL_6,
    35: adc_channel_t.ADC_CHANNEL_7,
}

def validate_adc_pin(value):
    if CORE.is_esp32:
        conf = pins.internal_gpio_input_pin_schema(value)
        value = conf[CONF_NUMBER]
        variant = get_esp32_variant()
        if variant != VARIANT_ESP32:
            raise cv.Invalid(f"This ESP32 variant ({variant}) is not supported")

        if value not in ADC_PIN_TO_CHANNEL:
            raise cv.Invalid(f"{variant} doesn't support ADC on this pin")

        return conf

    raise NotImplementedError

def validate_config(config):
    adc_frequency = config.get(CONF_FREQUENCY) * config.get(CONF_OVERSAMPLING)
    if adc_frequency < 20000:
        raise cv.Invalid("Real ADC frequency (frequency * oversampling) must be greter than 20kHz")
    if adc_frequency > 2000000:
        raise cv.Invalid("Real ADC frequency (frequency * oversampling) must be lower than 2MHz")

    return config

fast_adc_ns = cg.esphome_ns.namespace("fast_adc")
FastADCComponent = fast_adc_ns.class_("FastADCComponent", cg.Component)

CONF_OVERSAMPLING = "oversampling"
CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(FastADCComponent),
            cv.Required(CONF_PIN): validate_adc_pin,
            cv.Required(CONF_FREQUENCY): cv.All(cv.frequency, cv.int_range(min=10, max=200000)),
            cv.Optional(CONF_OVERSAMPLING, default=20): cv.int_range(min=10, max=2000),
            cv.Optional(CONF_CALIBRATION): cv.ensure_list(cv.positive_float),
        }
    )
    .extend(cv.COMPONENT_SCHEMA),
    cv.only_on_esp32,
    esp32.only_on_variant(supported=[esp32.const.VARIANT_ESP32]),
    validate_config,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    pin = await cg.gpio_pin_expression(config[CONF_PIN])
    cg.add(var.set_pin(pin))

    adc_pin_num = config[CONF_PIN][CONF_NUMBER]
    adc_channel = ADC_PIN_TO_CHANNEL[adc_pin_num]
    cg.add(var.set_channel(adc_channel))

    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    cg.add(var.set_oversampling(config[CONF_OVERSAMPLING]))
    
    if CONF_CALIBRATION in config:
        cg.add(var.set_calibration_data(config[CONF_CALIBRATION]))
        cg.add_build_flag("-DUSE_EXTERNAL_CALUBRATION")