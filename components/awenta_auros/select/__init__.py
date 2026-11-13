import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.types import ConfigType

from .. import CONF_AWENTA_AUROS_ID, AwentaAurosComponent, awenta_auros_ns

DEPENDENCIES = ["awenta_auros"]

FanModeSelect = awenta_auros_ns.class_("FanModeSelect", select.Select)
ThermostatModeSelect = awenta_auros_ns.class_("ThermostatModeSelect", select.Select)
BypassModeSelect = awenta_auros_ns.class_("BypassModeSelect", select.Select)

CONF_FAN_MODE = "fan_mode"
CONF_THERMOSTAT_MODE = "thermostat_mode"
CONF_BYPASS_MODE = "bypass_mode"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(cg.EntityBase),
        cv.GenerateID(CONF_AWENTA_AUROS_ID): cv.use_id(AwentaAurosComponent),
        cv.Optional(CONF_FAN_MODE): select.select_schema(
            FanModeSelect,
            icon="mdi:fan",
        ),
        cv.Optional(CONF_THERMOSTAT_MODE): select.select_schema(
            ThermostatModeSelect,
            icon="mdi:thermostat",
        ),
        cv.Optional(CONF_BYPASS_MODE): select.select_schema(
            BypassModeSelect,
            icon="mdi:valve",
        ),
    }
)


async def to_code(config: ConfigType) -> None:
    hub = await cg.get_variable(config[CONF_AWENTA_AUROS_ID])
    if conf := config.get(CONF_FAN_MODE):
        sel = await select.new_select(
            conf,
            options=["Off", "Speed 1", "Speed 2", "Speed 3", "Party", "Schedule"],
        )
        await cg.register_parented(sel, hub)
        cg.add(hub.set_fan_mode_select(sel))
    if conf := config.get(CONF_THERMOSTAT_MODE):
        sel = await select.new_select(
            conf,
            options=["Schedule", "Party", "Manual"],
        )
        await cg.register_parented(sel, hub)
        cg.add(hub.set_thermostat_mode_select(sel))
    if conf := config.get(CONF_BYPASS_MODE):
        sel = await select.new_select(
            conf,
            options=["Off", "Auto", "On"],
        )
        await cg.register_parented(sel, hub)
        cg.add(hub.set_bypass_mode_select(sel))
