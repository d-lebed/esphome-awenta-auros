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

from .. import CONF_AWENTA_AUROS_ID, AwentaAurosComponent, awenta_auros_ns

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
SmoothFanSpeedNumber = awenta_auros_ns.class_("SmoothFanSpeedNumber", number.Number)

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
CONF_SMOOTH_FAN_SPEED = "smooth_fan_speed"

CONF_SFS_MIN_SPEED = "sfs_min_speed"
CONF_SFS_MAX_SPEED = "sfs_max_speed"
CONF_SFS_SPEED_1_DEFAULT = "sfs_speed_1_default"
CONF_SFS_SPEED_2_DEFAULT = "sfs_speed_2_default"
CONF_SFS_SPEED_3_DEFAULT = "sfs_speed_3_default"


def _validate_sfs_config(config: ConfigType) -> ConfigType:
    min_spd = config[CONF_SFS_MIN_SPEED]
    max_spd = config[CONF_SFS_MAX_SPEED]
    def_1 = config[CONF_SFS_SPEED_1_DEFAULT]
    def_2 = config[CONF_SFS_SPEED_2_DEFAULT]
    def_3 = config[CONF_SFS_SPEED_3_DEFAULT]

    if min_spd >= max_spd:
        raise cv.Invalid(
            f"sfs_min_speed ({min_spd}) must be strictly less than sfs_max_speed ({max_spd})"
        )
    if not (def_1 < def_2 < def_3):
        raise cv.Invalid(
            "Default speeds must be strictly ordered: "
            f"sfs_speed_1_default ({def_1}) < sfs_speed_2_default ({def_2}) < sfs_speed_3_default ({def_3})"
        )
    if def_1 < min_spd or def_3 > max_spd:
        raise cv.Invalid(
            f"Default speeds ({def_1}, {def_2}, {def_3}) must be within "
            f"[sfs_min_speed ({min_spd}) .. sfs_max_speed ({max_spd})]"
        )
    return config


SMOOTH_FAN_SPEED_SCHEMA = cv.All(
    number.number_schema(
        SmoothFanSpeedNumber,
        unit_of_measurement=UNIT_PERCENT,
        icon="mdi:fan",
    ).extend(
        {
            cv.Optional(CONF_SFS_MIN_SPEED, default=1): cv.int_range(min=1, max=98),
            cv.Optional(CONF_SFS_MAX_SPEED, default=99): cv.int_range(min=2, max=99),
            cv.Optional(CONF_SFS_SPEED_1_DEFAULT, default=30): cv.int_range(min=0, max=100),
            cv.Optional(CONF_SFS_SPEED_2_DEFAULT, default=60): cv.int_range(min=0, max=100),
            cv.Optional(CONF_SFS_SPEED_3_DEFAULT, default=90): cv.int_range(min=0, max=100),
        }
    ),
    _validate_sfs_config,
)

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
        cv.Optional(CONF_SMOOTH_FAN_SPEED): SMOOTH_FAN_SPEED_SCHEMA,
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

    if conf := config.get(CONF_SMOOTH_FAN_SPEED):
        num = await number.new_number(conf, min_value=0.0, max_value=100.0, step=1.0)
        await cg.register_parented(num, hub)
        cg.add(hub.set_smooth_fan_speed_number(num))
        cg.add(
            hub.configure_smooth_fan_speed(
                conf[CONF_SFS_MIN_SPEED],
                conf[CONF_SFS_MAX_SPEED],
                conf[CONF_SFS_SPEED_1_DEFAULT],
                conf[CONF_SFS_SPEED_2_DEFAULT],
                conf[CONF_SFS_SPEED_3_DEFAULT],
            )
        )
