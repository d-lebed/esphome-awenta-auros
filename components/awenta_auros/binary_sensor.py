import esphome.codegen as cg
from esphome.components import binary_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.types import ConfigType

from . import CONF_AWENTA_AUROS_ID, AwentaAurosComponent

DEPENDENCIES = ["awenta_auros"]

CONF_COOLING_DEMAND = "cooling_demand"
CONF_VENTILATION_MODE = "ventilation_mode"
CONF_BYPASS_POSITION = "bypass_position"
CONF_AWAY = "away"
CONF_FIREPLACE_MODE = "fireplace_mode"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(cg.EntityBase),
        cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        cv.Optional(CONF_COOLING_DEMAND): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_VENTILATION_MODE): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_BYPASS_POSITION): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_AWAY): binary_sensor.binary_sensor_schema(),
        cv.Optional(CONF_FIREPLACE_MODE): binary_sensor.binary_sensor_schema(
            icon="mdi:fireplace"
        ),
    }
)


async def to_code(config: ConfigType) -> None:
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    for key in (
        CONF_COOLING_DEMAND,
        CONF_VENTILATION_MODE,
        CONF_BYPASS_POSITION,
        CONF_AWAY,
        CONF_FIREPLACE_MODE,
    ):
        if conf := config.get(key):
            sens = await binary_sensor.new_binary_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_binary_sensor")(sens))
