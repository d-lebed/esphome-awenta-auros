import esphome.codegen as cg
from esphome.components import climate
import esphome.config_validation as cv
from esphome.types import ConfigType

from .. import CONF_AWENTA_AUROS_ID, AwentaAurosComponent, awenta_auros_ns

DEPENDENCIES = ["awenta_auros"]

AwentaAurosClimate = awenta_auros_ns.class_(
    "AwentaAurosClimate", climate.Climate, cg.Component
)

CONFIG_SCHEMA = (
    climate.climate_schema(AwentaAurosClimate, icon="mdi:air-filter")
    .extend(
        {
            cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config: ConfigType) -> None:
    cg.add_define("USE_AWENTA_AUROS_CLIMATE")
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    var = await climate.new_climate(config)
    await cg.register_component(var, config)
    cg.add(hub.set_climate(var))
