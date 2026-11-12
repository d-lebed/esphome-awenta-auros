import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_HUMIDITY,
    DEVICE_CLASS_SPEED,
    DEVICE_CLASS_TEMPERATURE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)
from esphome.types import ConfigType

from . import CONF_AWENTA_AUROS_ID, AwentaAurosComponent

DEPENDENCIES = ["awenta_auros"]

CONF_TARGET_TEMPERATURE = "target_temperature"
CONF_ROOM_TEMPERATURE = "room_temperature"
CONF_ROOM_HUMIDITY = "room_humidity"
CONF_INTAKE_TEMPERATURE = "intake_temperature"
CONF_SUPPLY_TEMPERATURE = "supply_temperature"
CONF_PREHEATER_TEMPERATURE = "preheater_temperature"
CONF_SUPPLY_TARGET_TEMPERATURE = "supply_target_temperature"
CONF_EXHAUST_TEMPERATURE = "exhaust_temperature"
CONF_EXTRACT_TEMPERATURE = "extract_temperature"
CONF_TARGET_FAN_SPEED = "target_fan_speed"
CONF_INTAKE_FAN_SPEED = "intake_fan_speed"
CONF_SUPPLY_FAN_SPEED = "supply_fan_speed"
CONF_EXTRACT_FAN_SPEED = "extract_fan_speed"
CONF_EXHAUST_FAN_SPEED = "exhaust_fan_speed"

CONF_CONFIG_INTAKE_TEMPERATURE = "config_intake_temperature"
CONF_CONFIG_COMFORT_TEMPERATURE = "config_comfort_temperature"
CONF_CONFIG_ECO_WINTER_TEMPERATURE = "config_eco_winter_temperature"
CONF_CONFIG_ECO_COOLING_TEMPERATURE = "config_eco_cooling_temperature"
CONF_CONFIG_MANUAL_TEMPERATURE = "config_manual_temperature"
CONF_CONFIG_AWAY_TEMPERATURE = "config_away_temperature"
CONF_CONFIG_PREHEATER_MIN_TEMPERATURE = "config_preheater_min_temperature"
CONF_CONFIG_SUPPLY_FAN_SPEED_1 = "config_supply_fan_speed_1"
CONF_CONFIG_SUPPLY_FAN_SPEED_2 = "config_supply_fan_speed_2"
CONF_CONFIG_SUPPLY_FAN_SPEED_3 = "config_supply_fan_speed_3"
CONF_CONFIG_SUPPLY_FAN_AIRING_SPEED = "config_supply_fan_airing_speed"
CONF_CONFIG_EXHAUST_FAN_SPEED_1 = "config_exhaust_fan_speed_1"
CONF_CONFIG_EXHAUST_FAN_SPEED_2 = "config_exhaust_fan_speed_2"
CONF_CONFIG_EXHAUST_FAN_SPEED_3 = "config_exhaust_fan_speed_3"
CONF_CONFIG_EXHAUST_FAN_AIRING_SPEED = "config_exhaust_fan_airing_speed"


def _temp_schema() -> cv.Schema:
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _diagnostic_temp_schema() -> cv.Schema:
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )


def _speed_schema() -> cv.Schema:
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_SPEED,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _diagnostic_speed_schema(icon: str) -> cv.Schema:
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_SPEED,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon=icon,
    )


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(cg.EntityBase),
        cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        cv.Optional(CONF_TARGET_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_ROOM_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_ROOM_HUMIDITY): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            accuracy_decimals=0,
            device_class=DEVICE_CLASS_HUMIDITY,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_INTAKE_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_SUPPLY_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_PREHEATER_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_SUPPLY_TARGET_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_EXHAUST_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_EXTRACT_TEMPERATURE): _temp_schema(),
        cv.Optional(CONF_TARGET_FAN_SPEED): _speed_schema(),
        cv.Optional(CONF_INTAKE_FAN_SPEED): _speed_schema(),
        cv.Optional(CONF_SUPPLY_FAN_SPEED): _speed_schema(),
        cv.Optional(CONF_EXTRACT_FAN_SPEED): _speed_schema(),
        cv.Optional(CONF_EXHAUST_FAN_SPEED): _speed_schema(),
        cv.Optional(CONF_CONFIG_INTAKE_TEMPERATURE): _diagnostic_temp_schema(),
        cv.Optional(CONF_CONFIG_COMFORT_TEMPERATURE): _diagnostic_temp_schema(),
        cv.Optional(CONF_CONFIG_ECO_WINTER_TEMPERATURE): _diagnostic_temp_schema(),
        cv.Optional(CONF_CONFIG_ECO_COOLING_TEMPERATURE): _diagnostic_temp_schema(),
        cv.Optional(CONF_CONFIG_MANUAL_TEMPERATURE): _diagnostic_temp_schema(),
        cv.Optional(CONF_CONFIG_AWAY_TEMPERATURE): _diagnostic_temp_schema(),
        cv.Optional(CONF_CONFIG_PREHEATER_MIN_TEMPERATURE): _diagnostic_temp_schema(),
        cv.Optional(CONF_CONFIG_SUPPLY_FAN_SPEED_1): _diagnostic_speed_schema("mdi:fan-speed-1"),
        cv.Optional(CONF_CONFIG_SUPPLY_FAN_SPEED_2): _diagnostic_speed_schema("mdi:fan-speed-2"),
        cv.Optional(CONF_CONFIG_SUPPLY_FAN_SPEED_3): _diagnostic_speed_schema("mdi:fan-speed-3"),
        cv.Optional(CONF_CONFIG_SUPPLY_FAN_AIRING_SPEED): _diagnostic_speed_schema("mdi:fan-plus"),
        cv.Optional(CONF_CONFIG_EXHAUST_FAN_SPEED_1): _diagnostic_speed_schema("mdi:fan-speed-1"),
        cv.Optional(CONF_CONFIG_EXHAUST_FAN_SPEED_2): _diagnostic_speed_schema("mdi:fan-speed-2"),
        cv.Optional(CONF_CONFIG_EXHAUST_FAN_SPEED_3): _diagnostic_speed_schema("mdi:fan-speed-3"),
        cv.Optional(CONF_CONFIG_EXHAUST_FAN_AIRING_SPEED): _diagnostic_speed_schema("mdi:fan-plus"),
    }
)


async def to_code(config: ConfigType) -> None:
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    for key in (
        CONF_TARGET_TEMPERATURE,
        CONF_ROOM_TEMPERATURE,
        CONF_ROOM_HUMIDITY,
        CONF_INTAKE_TEMPERATURE,
        CONF_SUPPLY_TEMPERATURE,
        CONF_PREHEATER_TEMPERATURE,
        CONF_SUPPLY_TARGET_TEMPERATURE,
        CONF_EXHAUST_TEMPERATURE,
        CONF_EXTRACT_TEMPERATURE,
        CONF_TARGET_FAN_SPEED,
        CONF_INTAKE_FAN_SPEED,
        CONF_SUPPLY_FAN_SPEED,
        CONF_EXTRACT_FAN_SPEED,
        CONF_EXHAUST_FAN_SPEED,
        CONF_CONFIG_INTAKE_TEMPERATURE,
        CONF_CONFIG_COMFORT_TEMPERATURE,
        CONF_CONFIG_ECO_WINTER_TEMPERATURE,
        CONF_CONFIG_ECO_COOLING_TEMPERATURE,
        CONF_CONFIG_MANUAL_TEMPERATURE,
        CONF_CONFIG_AWAY_TEMPERATURE,
        CONF_CONFIG_PREHEATER_MIN_TEMPERATURE,
        CONF_CONFIG_SUPPLY_FAN_SPEED_1,
        CONF_CONFIG_SUPPLY_FAN_SPEED_2,
        CONF_CONFIG_SUPPLY_FAN_SPEED_3,
        CONF_CONFIG_SUPPLY_FAN_AIRING_SPEED,
        CONF_CONFIG_EXHAUST_FAN_SPEED_1,
        CONF_CONFIG_EXHAUST_FAN_SPEED_2,
        CONF_CONFIG_EXHAUST_FAN_SPEED_3,
        CONF_CONFIG_EXHAUST_FAN_AIRING_SPEED,
    ):
        if conf := config.get(key):
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
