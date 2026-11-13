import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC
from esphome.types import ConfigType

from .. import CONF_AWENTA_AUROS_ID, AwentaAurosComponent

DEPENDENCIES = ["awenta_auros"]

CONF_COOLING_DEMAND = "cooling_demand"
CONF_BYPASS_POSITION = "bypass_position"
CONF_SCHEDULE_MODE = "schedule_mode"
CONF_CONFIG_AIRING = "config_airing"
CONF_CONFIG_AWAY = "config_away"
CONF_CONFIG_HOLIDAY = "config_holiday"
CONF_CONFIG_FIREPLACE_MODE = "config_fireplace_mode"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(cg.EntityBase),
        cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        cv.Optional(CONF_COOLING_DEMAND): binary_sensor.binary_sensor_schema(
            icon="mdi:snowflake",
        ),
        cv.Optional(CONF_BYPASS_POSITION): binary_sensor.binary_sensor_schema(
            icon="mdi:valve",
        ),
        cv.Optional(CONF_SCHEDULE_MODE): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:calendar-clock",
        ),
        cv.Optional(CONF_CONFIG_AIRING): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:weather-windy",
        ),
        cv.Optional(CONF_CONFIG_AWAY): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:home-export-outline",
        ),
        cv.Optional(CONF_CONFIG_HOLIDAY): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:beach",
        ),
        cv.Optional(CONF_CONFIG_FIREPLACE_MODE): binary_sensor.binary_sensor_schema(
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:fireplace",
        ),
    }
)


async def to_code(config: ConfigType) -> None:
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    for key in (
        CONF_COOLING_DEMAND,
        CONF_BYPASS_POSITION,
        CONF_SCHEDULE_MODE,
        CONF_CONFIG_AIRING,
        CONF_CONFIG_AWAY,
        CONF_CONFIG_HOLIDAY,
        CONF_CONFIG_FIREPLACE_MODE,
    ):
        if conf := config.get(key):
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
