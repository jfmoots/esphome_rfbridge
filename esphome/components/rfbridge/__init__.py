import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome.const import CONF_ID

dependencies = ["spi"]

rfbridge_ns = cg.esphome_ns.namespace("rfbridge")
RFBridgeComponent = rfbridge_ns.class_("RFBridgeComponent", cg.Component, spi.SPIDevice)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RFBridgeComponent),
        cv.Optional("cs_pin"): cv.Any(str, int),
        cv.Optional("gdo0_pin"): cv.Any(str, int),
        cv.Optional("gdo2_pin"): cv.Any(str, int),
    }
).extend(cv.COMPONENT_SCHEMA).extend(spi.spi_device_schema(cs_pin_required=True))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await spi.register_spi_device(var, config)
