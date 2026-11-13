import esphome.codegen as cg
from esphome.components import fan
import esphome.config_validation as cv
from esphome.types import ConfigType

from .. import CONF_AWENTA_AUROS_ID, AwentaAurosComponent, awenta_auros_ns

DEPENDENCIES = ["awenta_auros"]

AwentaAurosFan = awenta_auros_ns.class_("AwentaAurosFan", fan.Fan, cg.Component)

CONFIG_SCHEMA = (
    fan.fan_schema(AwentaAurosFan, icon="mdi:fan", default_restore_mode="NO_RESTORE")
    .extend(
        {
            cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config: ConfigType) -> None:
    cg.add_define("USE_AWENTA_AUROS_FAN")
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    var = await fan.new_fan(config)
    await cg.register_component(var, config)
    cg.add(hub.set_fan(var))
