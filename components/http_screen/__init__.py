import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, http_request
from esphome.const import CONF_ID

DEPENDENCIES = ['display', 'http_request']

AUTO_LOAD = ["http_screen"]

mqtt_ns = cg.esphome_ns.namespace("http_screen")

HttpScreen = mqtt_ns.class_("HttpScreen", cg.Component)

CONF_SCREEN = "display"
CONF_HTTP_CLIENT = "http_client"

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(HttpScreen),
    cv.Required(CONF_SCREEN): cv.use_id(display.DisplayBuffer),
    cv.Required(CONF_HTTP_CLIENT): cv.use_id(http_request.HttpRequestComponent),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    screen = await cg.get_variable(config[CONF_SCREEN])
    httpClient = await cg.get_variable(config[CONF_HTTP_CLIENT])

    cg.add(var.set_display(screen))
    cg.add(var.set_http_client(httpClient))

    await cg.register_component(var, config)