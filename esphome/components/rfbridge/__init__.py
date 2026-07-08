import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins, automation
from esphome.const import CONF_ID

CONF_CS_PIN = "cs_pin"
CONF_SCK_PIN = "sck_pin"
CONF_MOSI_PIN = "mosi_pin"
CONF_MISO_PIN = "miso_pin"
CONF_GDO0_PIN = "gdo0_pin"
CONF_GDO2_PIN = "gdo2_pin"
CONF_DIAGNOSTIC_LOGGING = "diagnostic_logging"
CONF_LOW24 = "low24"
CONF_REMOTE_ID = "remote_id"
CONF_REPEATS = "repeats"

rfbridge_ns = cg.esphome_ns.namespace("rfbridge")
RFBridgeComponent = rfbridge_ns.class_("RFBridgeComponent", cg.Component)
SendOutprizeLow24Action = rfbridge_ns.class_("SendOutprizeLow24Action", automation.Action)
SendOutprizePowerOffAction = rfbridge_ns.class_("SendOutprizePowerOffAction", automation.Action)
SendOutprizeFanOffAction = rfbridge_ns.class_("SendOutprizeFanOffAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RFBridgeComponent),
        cv.Required(CONF_CS_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_SCK_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_MOSI_PIN): pins.gpio_output_pin_schema,
        cv.Required(CONF_MISO_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_GDO0_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_GDO2_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_DIAGNOSTIC_LOGGING, default=False): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_cs_pin(await cg.gpio_pin_expression(config[CONF_CS_PIN])))
    cg.add(var.set_sck_pin(await cg.gpio_pin_expression(config[CONF_SCK_PIN])))
    cg.add(var.set_mosi_pin(await cg.gpio_pin_expression(config[CONF_MOSI_PIN])))
    cg.add(var.set_miso_pin(await cg.gpio_pin_expression(config[CONF_MISO_PIN])))

    if CONF_GDO0_PIN in config:
        cg.add(var.set_gdo0_pin(await cg.gpio_pin_expression(config[CONF_GDO0_PIN])))

    if CONF_GDO2_PIN in config:
        cg.add(var.set_gdo2_pin(await cg.gpio_pin_expression(config[CONF_GDO2_PIN])))

    cg.add(var.set_diagnostic_logging(config[CONF_DIAGNOSTIC_LOGGING]))


OUTPRIZE_ACTION_BASE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(RFBridgeComponent),
        cv.Optional(CONF_REMOTE_ID, default=0x6CF): cv.templatable(cv.hex_uint32_t),
        cv.Optional(CONF_REPEATS, default=3): cv.templatable(cv.int_range(min=1, max=10)),
    }
)


@automation.register_action(
    "rfbridge.send_outprize_low24",
    SendOutprizeLow24Action,
    OUTPRIZE_ACTION_BASE_SCHEMA.extend(
        {
            cv.Required(CONF_LOW24): cv.templatable(cv.hex_uint32_t),
        }
    ),
)
async def send_outprize_low24_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    remote_id = await cg.templatable(config[CONF_REMOTE_ID], args, cg.uint32)
    cg.add(var.set_remote_id(remote_id))

    low24 = await cg.templatable(config[CONF_LOW24], args, cg.uint32)
    cg.add(var.set_low24(low24))

    repeats = await cg.templatable(config[CONF_REPEATS], args, cg.uint8)
    cg.add(var.set_repeats(repeats))

    return var


@automation.register_action(
    "rfbridge.send_outprize_power_off",
    SendOutprizePowerOffAction,
    OUTPRIZE_ACTION_BASE_SCHEMA,
)
async def send_outprize_power_off_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    remote_id = await cg.templatable(config[CONF_REMOTE_ID], args, cg.uint32)
    cg.add(var.set_remote_id(remote_id))

    repeats = await cg.templatable(config[CONF_REPEATS], args, cg.uint8)
    cg.add(var.set_repeats(repeats))

    return var


@automation.register_action(
    "rfbridge.send_outprize_fan_off",
    SendOutprizeFanOffAction,
    OUTPRIZE_ACTION_BASE_SCHEMA,
)
async def send_outprize_fan_off_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])

    remote_id = await cg.templatable(config[CONF_REMOTE_ID], args, cg.uint32)
    cg.add(var.set_remote_id(remote_id))

    repeats = await cg.templatable(config[CONF_REPEATS], args, cg.uint8)
    cg.add(var.set_repeats(repeats))

    return var
