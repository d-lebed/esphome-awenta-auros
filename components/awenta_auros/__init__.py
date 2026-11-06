from esphome import pins
import esphome.codegen as cg
from esphome.components import time, uart
import esphome.config_validation as cv
from esphome.const import CONF_FLOW_CONTROL_PIN, CONF_ID, CONF_TIME_ID
from esphome.types import ConfigType

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@d-lebed"]
DOMAIN = "awenta_auros"
MULTI_CONF = True

CONF_AWENTA_AUROS_ID = "awenta_auros_id"

awenta_auros_ns = cg.esphome_ns.namespace("awenta_auros")
AwentaAurosComponent = awenta_auros_ns.class_(
    "AwentaAurosComponent", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AwentaAurosComponent),
            cv.Optional(CONF_FLOW_CONTROL_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config: ConfigType) -> None:
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
    if flow_control_pin_config := config.get(CONF_FLOW_CONTROL_PIN):
        pin = await cg.gpio_pin_expression(flow_control_pin_config)
        cg.add(var.set_flow_control_pin(pin))
    if time_id := config.get(CONF_TIME_ID):
        time_comp = await cg.get_variable(time_id)
        cg.add(var.set_time(time_comp))
