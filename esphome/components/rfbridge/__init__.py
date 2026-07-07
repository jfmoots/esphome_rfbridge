import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID

CONF_CS_PIN = "cs_pin"
CONF_GDO0_PIN = "gdo0_pin"
CONF_GDO2_PIN = "gdo2_pin"
CONF_SCK_PIN = "sck_pin"
CONF_MOSI_PIN = "mosi_pin"
CONF_MISO_PIN = "miso_pin"

rfbridge_ns = cg.esphome_ns.namespace("rfbridge")
RFBridgeComponent = rfbridge_ns.class_("RFBridgeComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RFBridgeComponent),
        cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_SCK_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_MOSI_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_MISO_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_GDO0_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_GDO2_PIN): pins.gpio_input_pin_schema,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cs = await cg.gpio_pin_expression(config[CONF_CS_PIN])
    cg.add(var.set_cs_pin(cs))

    sck = await cg.gpio_pin_expression(config[CONF_SCK_PIN])
    cg.add(var.set_sck_pin(sck))

    mosi = await cg.gpio_pin_expression(config[CONF_MOSI_PIN])
    cg.add(var.set_mosi_pin(mosi))

    miso = await cg.gpio_pin_expression(config[CONF_MISO_PIN])
    cg.add(var.set_miso_pin(miso))

    if CONF_GDO0_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_GDO0_PIN])
        cg.add(var.set_gdo0_pin(pin))

    if CONF_GDO2_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_GDO2_PIN])
        cg.add(var.set_gdo2_pin(pin))
