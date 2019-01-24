import streams
import threading
import rtc

from riverdi.displays.bt81x import ctp50
from bridgetek.bt81x import bt81x

def updating_clock():
    tt = rtc.get_utc()
    bt81x.add_clock(bt81x.display_conf.width//2, (bt81x.display_conf.height*5)//7, bt81x.display_conf.height//6, 
        0, tt.tm_hour, tt.tm_min, tt.tm_sec, 0)

    # simulate a touch event to refresh the clock widget, sleep is needed to not let other threads starve
    sleep(400)
    widget_choice_evt.set()

def demo_spinner(style):
    bt81x.spinner(bt81x.display_conf.width//2, (bt81x.display_conf.height*5)//7, style, 0)

# the widgets tuple contains tuples with:
#   - widget description
#   - widget draw function
#   - widget draw function arguments
widgets = (
    ("spinner clock", demo_spinner, bt81x.SPINNER_CLOCK),
    ("spinner orbiting", demo_spinner, bt81x.SPINNER_ORBITING),
    ("spinner circle", demo_spinner, bt81x.SPINNER_CIRCLE),
    ("spinner line", demo_spinner, bt81x.SPINNER_LINE),
    ("auto-updating clock", updating_clock, ),
)

widget_choice = None
widget_choice_evt = threading.Event()
widget_selection_keys = "123456789"
widget_selection_keys = widget_selection_keys[:len(widgets)]

def widget_choice_cbk(_widget_choice, _):
    global widget_choice
    widget_choice = _widget_choice - __ORD('0')
    widget_choice_evt.set()

palette_default = bt81x.Palette((0xff, 0xff, 0xff), foreground=(0x3c, 0x82, 0x82))
txt = bt81x.Text(0, 0, 31, bt81x.OPT_CENTERX | bt81x.OPT_CENTERY, "", palette=palette_default)

streams.serial() # open serial channel to display debug messages

print('> Init chip')
bt81x.init(SPI0, D4, D5, D34)
bt81x.touch_loop(((-1, widget_choice_cbk), )) # listen to touch events and make widget_choice_cbk process them

def widget_selection():
    bt81x.dl_start()
    bt81x.clear(1, 1, 1)

    txt.font = 31
    txt.text = "Choose a widget"
    txt.x = bt81x.display_conf.width//2
    txt.y = (bt81x.display_conf.height*2)//11
    bt81x.add_text(txt)

    bt81x.track(bt81x.display_conf.width//5, (bt81x.display_conf.height*3)//11, 500, 100, 0)
    bt81x.add_keys(bt81x.display_conf.width//5, (bt81x.display_conf.height*3)//11, 500, 100, 31, 0, widget_selection_keys)

    if widget_choice:
        print('> displaying', widgets[widget_choice-1][0])
        widgets[widget_choice-1][1](*widgets[widget_choice-1][2:])

    bt81x.display()
    bt81x.swap_and_empty()

    widget_choice_evt.wait()
    widget_choice_evt.clear()

while True:
    widget_selection()