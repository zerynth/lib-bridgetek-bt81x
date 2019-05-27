# display networks

import streams
from wireless import wifi

from espressif.esp32net import esp32wifi as wifi_driver 

from riverdi.displays.bt81x import ctp50
from bridgetek.bt81x import bt81x

streams.serial()
wifi_driver.auto_init()

TAG_BASE = 1
BTN_WIDTH = bt81x.display_conf.width
BTN_HEIGHT = bt81x.display_conf.height // 8
MAX_NET = 7

# different colors for pressed and non-pressed buttons
palette_default = bt81x.Palette((0xff, 0xff, 0xff), (0, 0, 0xff))
palette_pressed = bt81x.Palette((0xff, 0xff, 0xff), (0xff, 0, 0))
btn = bt81x.Button(0, 0, BTN_WIDTH, BTN_HEIGHT, 31, 0, "", palette=palette_default)

def draw_networks(networks, pressed=None):
    bt81x.dl_start()
    bt81x.clear(1, 1, 1)

    for i, ssid_sec in enumerate(networks):
        if i == MAX_NET:
            # do not display more networks than MAX_NET
            break
        btn.palette = palette_pressed if i==pressed else palette_default
        btn.text = ("%s::%s") % (ssid_sec[0], ssid_sec[1])
        btn.y = i*BTN_HEIGHT
        bt81x.track(0, btn.y, BTN_WIDTH, BTN_HEIGHT, TAG_BASE + i)
        bt81x.tag(TAG_BASE + i)
        bt81x.add_button(btn)

    bt81x.display()
    bt81x.swap_and_empty()

def pressed(tag, tracked, tp):
    print("PRESSED!", tag)
    draw_networks(networks, pressed=tag-1)

bt81x.init(SPI0, D4, D33, D34)

# uncomment these lines to calibrate resistive displays
# bt81x.dl_start()
# bt81x.calibrate()
# bt81x.swap_and_empty()

wifi_sec=("Open","WEP","WPA","WPA2")
print("> scanning")
# scan and keep network name and security
networks = [(scanned[0], wifi_sec[scanned[1]]) for scanned in wifi.scan(15000)]
print("> scan results...")

draw_networks(networks)
# register 'pressed' callback for touch events on every tagged object
bt81x.touch_loop(((-1, pressed), ))
