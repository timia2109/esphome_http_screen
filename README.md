# esphome_http_screen
A http based image lib for esphome.

Only works with black/white images

## Usage example
```yaml
external_components:
  - source: github://timia2109/esphome_http_screen
    components: [ http_screen ]

# Example configuration entry
http_request:
  useragent: esphome/screen
  timeout: 10s
  id: http_client

# 480x800
display:
  - platform: waveshare_epaper
    id: epaper
    rotation: 90
    cs_pin: GPIO15
    dc_pin: GPIO27
    busy_pin: GPIO25
    reset_pin: GPIO26
    model: 7.50inV2alt
    update_interval: never
    reset_duration: 2ms

mqtt:
  broker: example.com
  username: !secret mqtt_user
  password: !secret mqtt_pass
  id: mqttClient
  discovery: false
  discovery_retain: false
  on_message:
    topic: esphome/screen
    qos: 0
    then:
      - lambda: !lambda |-
          id(httpScreen).load_image(x);

http_screen:
  id: httpScreen
  display: epaper
  http_client: http_client
```

## Compression

I'm using a custom compression for transferring images to the esp, which only works for black/white images.

If you want to use the ESP part and omit the server, the spec is as follows

 - The url must serve base64 due to http client drawbacks in esphome
 - Data is represented as **unsigned short** (16 bit)
 - Transport as least significant bit first
 - Drawing starts at (1,1) and runs line by line (increasing X)
 - The two highest bits represent the meaning of the following bits
   - 0 = plain mode
   - 1 = white fill
   - 2 = black fill
 - The next bits represent the payload
   - For plain mode, it toggles each pixel, where 1 means black and 0 means white (read from high to low).
   - For white and black fill, it represents the number of pixels filled with the color.