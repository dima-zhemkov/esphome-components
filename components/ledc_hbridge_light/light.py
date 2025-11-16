import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.components.ledc.output import LEDCOutput
from esphome.const import (
    CONF_CONSTANT_BRIGHTNESS,
    CONF_OUTPUT_ID,
    CONF_COLD_WHITE,
    CONF_WARM_WHITE,
    CONF_COLD_WHITE_COLOR_TEMPERATURE,
    CONF_WARM_WHITE_COLOR_TEMPERATURE,
)

CODEOWNERS = ["@dima-zhemkov"]

DEPENDENCIES = ["esp32"]

CONF_MAX_FREQUENCY = "max_frequency"
CONF_MIN_FREQUENCY = "min_frequency"
CONF_MIN_PULSE = "min_pulse"
CONF_LOW_FREQ_BRIGHTNESS_CORRECTION = "low_frequency_brightness_correction"

ledc_hbridge_light_ns = cg.esphome_ns.namespace("ledc_hbridge_light")
LedcHbridgeLightOutput = ledc_hbridge_light_ns.class_("LedcHbridgeLightOutput", light.LightOutput)

CONFIG_SCHEMA = cv.All(
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(LedcHbridgeLightOutput),
            cv.Required(CONF_COLD_WHITE): cv.use_id(LEDCOutput),
            cv.Required(CONF_WARM_WHITE): cv.use_id(LEDCOutput),
            cv.Optional(CONF_COLD_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
            cv.Optional(CONF_WARM_WHITE_COLOR_TEMPERATURE): cv.color_temperature,
            cv.Optional(CONF_CONSTANT_BRIGHTNESS, default=False): cv.boolean,
            cv.Optional(CONF_MAX_FREQUENCY, default="19531Hz"): cv.frequency,
            cv.Optional(CONF_MIN_FREQUENCY, default="1220Hz"): cv.frequency,
            cv.Optional(CONF_MIN_PULSE, default="500ns"): cv.positive_time_period_nanoseconds,
            cv.Optional(CONF_LOW_FREQ_BRIGHTNESS_CORRECTION, default=1.0): cv.percentage,
        }
    ),
    cv.has_none_or_all_keys(
        [CONF_COLD_WHITE_COLOR_TEMPERATURE, CONF_WARM_WHITE_COLOR_TEMPERATURE]
    ),
    light.validate_color_temperature_channels,
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)

    cwhite = await cg.get_variable(config[CONF_COLD_WHITE])
    cg.add(var.set_cold_white(cwhite))
    if cold_white_color_temperature := config.get(CONF_COLD_WHITE_COLOR_TEMPERATURE):
        cg.add(var.set_cold_white_temperature(cold_white_color_temperature))

    wwhite = await cg.get_variable(config[CONF_WARM_WHITE])
    cg.add(var.set_warm_white(wwhite))
    if warm_white_color_temperature := config.get(CONF_WARM_WHITE_COLOR_TEMPERATURE):
        cg.add(var.set_warm_white_temperature(warm_white_color_temperature))

    cg.add(var.set_constant_brightness(config[CONF_CONSTANT_BRIGHTNESS]))
    cg.add(var.set_max_frequency(config[CONF_MAX_FREQUENCY]))
    cg.add(var.set_min_frequency(config[CONF_MIN_FREQUENCY]))
    cg.add(var.set_min_pulse(config[CONF_MIN_PULSE].total_nanoseconds / 1e9))
    cg.add(var.set_low_frequency_brightness_correction(config[CONF_LOW_FREQ_BRIGHTNESS_CORRECTION]))