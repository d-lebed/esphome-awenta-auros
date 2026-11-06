import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_TEMPERATURE,
    UNIT_CELSIUS,
)
from esphome.types import ConfigType

from . import CONF_AWENTA_AUROS_ID, AwentaAurosComponent, awenta_auros_ns

DEPENDENCIES = ["awenta_auros"]

ComfortTemperatureNumber = awenta_auros_ns.class_("ComfortTemperatureNumber", number.Number)
EcoWinterTemperatureNumber = awenta_auros_ns.class_("EcoWinterTemperatureNumber", number.Number)
EcoCoolingTemperatureNumber = awenta_auros_ns.class_("EcoCoolingTemperatureNumber", number.Number)
ManualTemperatureNumber = awenta_auros_ns.class_("ManualTemperatureNumber", number.Number)
AwayTemperatureNumber = awenta_auros_ns.class_("AwayTemperatureNumber", number.Number)

CONF_COMFORT_TEMPERATURE = "comfort_temperature"
CONF_ECO_WINTER_TEMPERATURE = "eco_winter_temperature"
CONF_ECO_COOLING_TEMPERATURE = "eco_cooling_temperature"
CONF_MANUAL_TEMPERATURE = "manual_temperature"
CONF_AWAY_TEMPERATURE = "away_temperature"

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
