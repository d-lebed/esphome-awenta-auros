import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID, DEVICE_CLASS_SWITCH
from esphome.types import ConfigType

from . import CONF_AWENTA_AUROS_ID, AwentaAurosComponent, awenta_auros_ns

DEPENDENCIES = ["awenta_auros"]

AiringSwitch = awenta_auros_ns.class_("AiringSwitch", switch.Switch)
FireplaceSwitch = awenta_auros_ns.class_("FireplaceSwitch", switch.Switch)
GheSwitch = awenta_auros_ns.class_("GheSwitch", switch.Switch)

CONF_AIRING = "airing"
CONF_FIREPLACE = "fireplace"
CONF_GHE = "ghe"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(cg.EntityBase),
        cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        cv.Optional(CONF_AIRING): switch.switch_schema(
            AiringSwitch,
            device_class=DEVICE_CLASS_SWITCH,
            icon="mdi:weather-windy",
        ),
        cv.Optional(CONF_FIREPLACE): switch.switch_schema(
            FireplaceSwitch,
            device_class=DEVICE_CLASS_SWITCH,
            icon="mdi:fireplace",
        ),
        cv.Optional(CONF_GHE): switch.switch_schema(
            GheSwitch,
            device_class=DEVICE_CLASS_SWITCH,
            icon="mdi:tunnel-outline",
        ),
    }
)


async def to_code(config: ConfigType) -> None:
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    if conf := config.get(CONF_AIRING):
        sw = await switch.new_switch(conf)
        await cg.register_parented(sw, hub)
        cg.add(hub.set_airing_switch(sw))
    if conf := config.get(CONF_FIREPLACE):
        sw = await switch.new_switch(conf)
        await cg.register_parented(sw, hub)
        cg.add(hub.set_fireplace_switch(sw))
    if conf := config.get(CONF_GHE):
        sw = await switch.new_switch(conf)
        await cg.register_parented(sw, hub)
        cg.add(hub.set_ghe_switch(sw))
