import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC
from esphome.types import ConfigType

from . import CONF_AWENTA_AUROS_ID, AwentaAurosComponent

DEPENDENCIES = ["awenta_auros"]

CONF_MASTER_FAN_SPEED = "master_fan_speed"
CONF_SEASON_MODE = "season_mode"
CONF_THERMOSTAT_MODE = "thermostat_mode"
CONF_CONFIG_FAN_SPEED = "config_fan_speed"
CONF_CONFIG_THERMOSTAT_MODE = "config_thermostat_mode"
CONF_CONFIG_BYPASS_MODE = "config_bypass_mode"
CONF_CONFIG_GHE_MODE = "config_ghe_mode"
CONF_TIME = "time"
CONF_DAY_OF_WEEK = "day_of_week"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(cg.EntityBase),
        cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        cv.Optional(CONF_MASTER_FAN_SPEED): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_SEASON_MODE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_THERMOSTAT_MODE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_CONFIG_FAN_SPEED): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_CONFIG_THERMOSTAT_MODE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_CONFIG_BYPASS_MODE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_CONFIG_GHE_MODE): text_sensor.text_sensor_schema(),
        cv.Optional(CONF_TIME): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_DAY_OF_WEEK): text_sensor.text_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
    }
)


async def to_code(config: ConfigType) -> None:
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    for key in (
        CONF_MASTER_FAN_SPEED,
        CONF_SEASON_MODE,
        CONF_THERMOSTAT_MODE,
        CONF_CONFIG_FAN_SPEED,
        CONF_CONFIG_THERMOSTAT_MODE,
        CONF_CONFIG_BYPASS_MODE,
        CONF_CONFIG_GHE_MODE,
        CONF_TIME,
        CONF_DAY_OF_WEEK,
    ):
        if conf := config.get(key):
            sens = await text_sensor.new_text_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
