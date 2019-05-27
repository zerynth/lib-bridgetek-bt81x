import streams
from riverdi.displays.bt81x import ctp50
from bridgetek.bt81x import bt81x

streams.serial()

# choose one logo to be loaded on flash
new_resource('zerynth_logo.bin')
# new_resource('zerynth_logo.png')

LOGO_W = 642
LOGO_H = 144
linestride = LOGO_W * 2 # with ARGB1555 and ARGB4 formats, 2 bytes per pixel

layout = (bt81x.ARGB1555, linestride) # choose this layout for zerynth_logo.bin
# layout = (bt81x.ARGB4, linestride)  # choose this layout for zerynth_logo.png

zerynth_logo = bt81x.Bitmap(1, 0, layout,
                    (bt81x.BILINEAR, bt81x.BORDER, bt81x.BORDER, LOGO_W, LOGO_H))

bt81x.init(SPI0, D4, D33, D34)

bt81x.inflate(0, 'zerynth_logo.bin')
# bt81x.load_image(0, 0, 'zerynth_logo.png')

bt81x.dl_start()
bt81x.clear_color(rgb=(0xff, 0xff, 0xff))
bt81x.clear(1, 1, 0)

zerynth_logo.prepare_draw()
zerynth_logo.draw(((bt81x.display_conf.width - LOGO_W)//2, (bt81x.display_conf.height - LOGO_H)//2), vertex_fmt=0)

bt81x.display()
bt81x.swap_and_empty()
