import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    ENTITY_CATEGORY_CONFIG,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)
from esphome.types import ConfigType

from . import CONF_AWENTA_AUROS_ID, AwentaAurosComponent, awenta_auros_ns

DEPENDENCIES = ["awenta_auros"]

ComfortTemperatureNumber = awenta_auros_ns.class_("ComfortTemperatureNumber", number.Number)
EcoWinterTemperatureNumber = awenta_auros_ns.class_("EcoWinterTemperatureNumber", number.Number)
EcoCoolingTemperatureNumber = awenta_auros_ns.class_("EcoCoolingTemperatureNumber", number.Number)
ManualTemperatureNumber = awenta_auros_ns.class_("ManualTemperatureNumber", number.Number)
AwayTemperatureNumber = awenta_auros_ns.class_("AwayTemperatureNumber", number.Number)
PreheaterMinTemperatureNumber = awenta_auros_ns.class_(
    "PreheaterMinTemperatureNumber", number.Number
)
SupplyFanSpeed1Number = awenta_auros_ns.class_("SupplyFanSpeed1Number", number.Number)
SupplyFanSpeed2Number = awenta_auros_ns.class_("SupplyFanSpeed2Number", number.Number)
SupplyFanSpeed3Number = awenta_auros_ns.class_("SupplyFanSpeed3Number", number.Number)
SupplyFanAiringSpeedNumber = awenta_auros_ns.class_("SupplyFanAiringSpeedNumber", number.Number)
ExhaustFanSpeed1Number = awenta_auros_ns.class_("ExhaustFanSpeed1Number", number.Number)
ExhaustFanSpeed2Number = awenta_auros_ns.class_("ExhaustFanSpeed2Number", number.Number)
ExhaustFanSpeed3Number = awenta_auros_ns.class_("ExhaustFanSpeed3Number", number.Number)
ExhaustFanAiringSpeedNumber = awenta_auros_ns.class_("ExhaustFanAiringSpeedNumber", number.Number)

CONF_COMFORT_TEMPERATURE = "comfort_temperature"
CONF_ECO_WINTER_TEMPERATURE = "eco_winter_temperature"
CONF_ECO_COOLING_TEMPERATURE = "eco_cooling_temperature"
CONF_MANUAL_TEMPERATURE = "manual_temperature"
CONF_AWAY_TEMPERATURE = "away_temperature"
CONF_PREHEATER_MIN_TEMPERATURE = "preheater_min_temperature"
CONF_SUPPLY_FAN_SPEED_1 = "supply_fan_speed_1"
CONF_SUPPLY_FAN_SPEED_2 = "supply_fan_speed_2"
CONF_SUPPLY_FAN_SPEED_3 = "supply_fan_speed_3"
CONF_SUPPLY_FAN_AIRING_SPEED = "supply_fan_airing_speed"
CONF_EXHAUST_FAN_SPEED_1 = "exhaust_fan_speed_1"
CONF_EXHAUST_FAN_SPEED_2 = "exhaust_fan_speed_2"
CONF_EXHAUST_FAN_SPEED_3 = "exhaust_fan_speed_3"
CONF_EXHAUST_FAN_AIRING_SPEED = "exhaust_fan_airing_speed"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(cg.EntityBase),
        cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        cv.Optional(CONF_COMFORT_TEMPERATURE): number.number_schema(
            ComfortTemperatureNumber,
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            icon="mdi:home-thermometer",
        ),
        cv.Optional(CONF_ECO_WINTER_TEMPERATURE): number.number_schema(
            EcoWinterTemperatureNumber,
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            icon="mdi:snowflake-thermometer",
        ),
        cv.Optional(CONF_ECO_COOLING_TEMPERATURE): number.number_schema(
            EcoCoolingTemperatureNumber,
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            icon="mdi:sun-thermometer",
        ),
        cv.Optional(CONF_MANUAL_TEMPERATURE): number.number_schema(
            ManualTemperatureNumber,
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            icon="mdi:thermometer-lines",
        ),
        cv.Optional(CONF_AWAY_TEMPERATURE): number.number_schema(
            AwayTemperatureNumber,
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            icon="mdi:home-export-outline",
        ),
        cv.Optional(CONF_PREHEATER_MIN_TEMPERATURE): number.number_schema(
            PreheaterMinTemperatureNumber,
            device_class=DEVICE_CLASS_TEMPERATURE,
            unit_of_measurement=UNIT_CELSIUS,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:heating-coil",
        ),
        cv.Optional(CONF_SUPPLY_FAN_SPEED_1): number.number_schema(
            SupplyFanSpeed1Number,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-speed-1",
        ),
        cv.Optional(CONF_SUPPLY_FAN_SPEED_2): number.number_schema(
            SupplyFanSpeed2Number,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-speed-2",
        ),
        cv.Optional(CONF_SUPPLY_FAN_SPEED_3): number.number_schema(
            SupplyFanSpeed3Number,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-speed-3",
        ),
        cv.Optional(CONF_SUPPLY_FAN_AIRING_SPEED): number.number_schema(
            SupplyFanAiringSpeedNumber,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-plus",
        ),
        cv.Optional(CONF_EXHAUST_FAN_SPEED_1): number.number_schema(
            ExhaustFanSpeed1Number,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-speed-1",
        ),
        cv.Optional(CONF_EXHAUST_FAN_SPEED_2): number.number_schema(
            ExhaustFanSpeed2Number,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-speed-2",
        ),
        cv.Optional(CONF_EXHAUST_FAN_SPEED_3): number.number_schema(
            ExhaustFanSpeed3Number,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-speed-3",
        ),
        cv.Optional(CONF_EXHAUST_FAN_AIRING_SPEED): number.number_schema(
            ExhaustFanAiringSpeedNumber,
            unit_of_measurement=UNIT_PERCENT,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:fan-plus",
        ),
    }
)


async def to_code(config: ConfigType) -> None:
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    for key, setter in (
        (CONF_COMFORT_TEMPERATURE, "set_comfort_temperature_number"),
        (CONF_ECO_WINTER_TEMPERATURE, "set_eco_winter_temperature_number"),
        (CONF_ECO_COOLING_TEMPERATURE, "set_eco_cooling_temperature_number"),
        (CONF_MANUAL_TEMPERATURE, "set_manual_temperature_number"),
        (CONF_AWAY_TEMPERATURE, "set_away_temperature_number"),
    ):
        if conf := config.get(key):
            num = await number.new_number(conf, min_value=10.0, max_value=30.0, step=0.1)
            await cg.register_parented(num, hub)
            cg.add(getattr(hub, setter)(num))

    if conf := config.get(CONF_PREHEATER_MIN_TEMPERATURE):
        num = await number.new_number(conf, min_value=-10.0, max_value=10.0, step=0.5)
        await cg.register_parented(num, hub)
        cg.add(hub.set_preheater_min_temperature_number(num))

    for key, setter in (
        (CONF_SUPPLY_FAN_SPEED_1, "set_supply_fan_speed_1_number"),
        (CONF_SUPPLY_FAN_SPEED_2, "set_supply_fan_speed_2_number"),
        (CONF_SUPPLY_FAN_SPEED_3, "set_supply_fan_speed_3_number"),
        (CONF_SUPPLY_FAN_AIRING_SPEED, "set_supply_fan_airing_speed_number"),
        (CONF_EXHAUST_FAN_SPEED_1, "set_exhaust_fan_speed_1_number"),
        (CONF_EXHAUST_FAN_SPEED_2, "set_exhaust_fan_speed_2_number"),
        (CONF_EXHAUST_FAN_SPEED_3, "set_exhaust_fan_speed_3_number"),
        (CONF_EXHAUST_FAN_AIRING_SPEED, "set_exhaust_fan_airing_speed_number"),
    ):
        if conf := config.get(key):
            num = await number.new_number(conf, min_value=0.0, max_value=100.0, step=1.0)
            await cg.register_parented(num, hub)
            cg.add(getattr(hub, setter)(num))
