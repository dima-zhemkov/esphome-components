import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light, output
from esphome.const import (
    CONF_CONSTANT_BRIGHTNESS,
    CONF_OUTPUT_ID,
    CONF_COLD_WHITE,
    CONF_WARM_WHITE,
    CONF_COLD_WHITE_COLOR_TEMPERATURE,
    CONF_WARM_WHITE_COLOR_TEMPERATURE,
)

CODEOWNERS = ["@dima-zhemkov"]

CONF_NIGHT_LIGHT = "night_light"
CONF_NIGHT_LIGHT_BRIGHTNESS_THRESHOLD = "night_light_brightness_threshold"
CONF_NIGHT_LIGHT_TURN_OFF_DELAY = "night_light_turn_off_delay"

yeelight_ceiling_ns = cg.esphome_ns.namespace("yeelight_ceiling")
YeelightCeiling = yeelight_ceiling_ns.class_("YeelightCeiling", light.LightOutput)

CONFIG_SCHEMA = cv.All(
    light.RGB_LIGHT_SCHEMA.extend(
        {
            cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(YeelightCeiling),
            cv.Required(CONF_COLD_WHITE): cv.use_id(output.FloatOutput),
            cv.Required(CONF_WARM_WHITE): cv.use_id(output.FloatOutput),
            cv.Required(CONF_NIGHT_LIGHT): cv.use_id(output.FloatOutput),
            cv.Optional(CONF_COLD_WHITE_COLOR_TEMPERATURE, default="6000K"): cv.color_temperature,
            cv.Optional(CONF_WARM_WHITE_COLOR_TEMPERATURE, default="2700K"): cv.color_temperature,
            cv.Optional(CONF_CONSTANT_BRIGHTNESS, default=True): cv.boolean,
            cv.Optional(CONF_NIGHT_LIGHT_BRIGHTNESS_THRESHOLD, default=0.105): cv.zero_to_one_float,
            cv.Optional(CONF_NIGHT_LIGHT_TURN_OFF_DELAY, default="80ms"): cv.positive_time_period_milliseconds,
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

    night_light = await cg.get_variable(config[CONF_NIGHT_LIGHT])
    cg.add(var.set_night_light(night_light))

    cg.add(var.set_constant_brightness(config[CONF_CONSTANT_BRIGHTNESS]))
    cg.add(var.set_night_light_brightness_threshold(config[CONF_NIGHT_LIGHT_BRIGHTNESS_THRESHOLD]))
    cg.add(var.set_night_light_turn_off_delay(config[CONF_NIGHT_LIGHT_TURN_OFF_DELAY]))