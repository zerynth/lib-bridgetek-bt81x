# -*- coding: utf-8 -*-
# @Author: lorenzo
# @Date:   2018-10-09 12:12:08
# @Last Modified by:   Lorenzo
# @Last Modified time: 2018-12-10 14:58:38

"""
.. module:: bt81x

*************
BT81x library
*************

This module exports classes and functions to handle Bridgetek BT81x family of Embedded Video Engines.

    """

import streams
import spi

class DisplayConf:
    """
.. class:: DisplayConf(width, height, \
                    hcycle, hoffset, hsync0, hsync1, \
                    vcycle, voffset, vsync0, vsync1, \
                    pclk, swizzle, pclkpol, cspread, dither, \
                    description)

    Class to store a display configuration.
    List of attributes:

        * :attr:`DisplayConf.width` display width in pixels
        * :attr:`DisplayConf.height` display height in pixels
        * :attr:`DisplayConf.hcycle` number of total PCLK cycles per horizontal line scan
        * :attr:`DisplayConf.hoffset` number of PCLK cycle before pixels are scanned out
        * :attr:`DisplayConf.hsync0` how many PCLK cycles for HSYNC0 during start of line
        * :attr:`DisplayConf.hsync1` how many PCLK cycles for HSYNC1 during start of line
        * :attr:`DisplayConf.vcycle` how many lines in one frame
        * :attr:`DisplayConf.voffset` how many lines taken after the start of a new frame
        * :attr:`DisplayConf.vsync0` how many lines of signal VSYNC0 takes at start of a new frame
        * :attr:`DisplayConf.vsync1`  how many lines of signal VSYNC1 takes at start of a new frame
        * :attr:`DisplayConf.pclk` main clock divider for PCLK, if 0 there is no PCLK output
        * :attr:`DisplayConf.swizzle` controls the arrangement of output RGB pins to support different LCD panels
        * :attr:`DisplayConf.pclkpol` ``0`` for PCLK polarity on the rising edge, `1` for falling edge
        * :attr:`DisplayConf.cspread` controls the transition of RGB signals with PCLK active clock edge
        * :attr:`DisplayConf.dither` ``1`` or ``0`` to respectively enable or disable dither
        * :attr:`DisplayConf.description` a string describing the display

    ``display_conf`` module global variable is automatically set with a :class:`DisplayConf` instance when importing a display module from a display vendor::

        from riverdi.displays.bt81x import ctp50
        from bridgetek.bt81x import bt81x

    """

    def __init__(self, width, height, 
                    hcycle, hoffset, hsync0, hsync1, 
                    vcycle, voffset, vsync0, vsync1, 
                    pclk, swizzle, pclkpol, cspread, dither,
                    description):
        self.width = width
        self.height = height

        self.hcycle = hcycle
        self.hoffset = hoffset
        self.hsync0 = hsync0
        self.hsync1 = hsync1

        self.vcycle = vcycle
        self.voffset = voffset
        self.vsync0 = vsync0
        self.vsync1 = vsync1

        self.pclk = pclk
        self.swizzle = swizzle
        self.pclkpol = pclkpol
        self.cspread = cspread
        self.dither = dither

        self.description = description

display_conf = None

@native_c("_bt81x_init", 
    [
        "csrc/bt81x_ifc.c",
        "csrc/riverdi-eve/app_layer/*",
        "csrc/riverdi-eve/eve_layer/*",
        "csrc/riverdi-eve/host_layer/zerynth/*",
        "#csrc/misc/zstdlib.c",
    ],
    [
        "EVE_3", 
        # using STM32_PLATFORM defines since they are ok for MCU platforms in general
        "STM32_PLATFORM",
        "STM32_PLATFORM_COCMD_BURST"
    ],
    [
        "-I.../csrc/riverdi-eve/app_layer",
        "-I.../csrc/riverdi-eve/eve_layer",
        "-I.../csrc/riverdi-eve/riverdi_modules",
        "-I.../csrc/riverdi-eve/host_layer/zerynth"
    ]
)
def _init(spi, cs, pd, int, spi_speed,
        width, height,
        hcycle, hoffset, hsync0, hsync1, 
        vcycle, voffset, vsync0, vsync1, 
        pclk, swizzle, pclkpol, cspread, dither):
    pass

def init(spi, cs, pd, int, dc=None, spi_speed=3000000):
    """
.. function:: init(spi, cs, pd, int, dc=None, spi_speed=3000000)

    :param spi: spi driver (``SPI0``, ``SPI1``, ``...``)
    :param cs: chip select pin
    :param pd: pd pin
    :param int: interrupt pin
    :param dc: display configuration as a :class:`DisplayConf` instance
    :param spi_speed: spi speed in Hertz

    Initializes the chip.

    When ``dc`` parameter is not specified ``display_conf`` global variable is used.
    """
    if dc is None:
        if display_conf is None:
            raise TypeError
        dc = display_conf
    _init(spi, cs, pd, int, spi_speed,
        dc.width, dc.height, 
        dc.hcycle, dc.hoffset, dc.hsync0, dc.hsync1, 
        dc.vcycle, dc.voffset, dc.vsync0, dc.vsync1, 
        dc.pclk, dc.swizzle, dc.pclkpol, dc.cspread, dc.dither
    )

class Palette:
    """
.. class:: Palette(font, foreground, background)

    Class to store a color palette for font, foreground and background.
    List of attributes:

        * :attr:`font` tuple of rgb values ``(r,g,b)``
        * :attr:`foreground` tuple of rgb values ``(r,g,b)``
        * :attr:`background` tuple of rgb values ``(r,g,b)``
    """
    def __init__(self, font, foreground=None, background=None):
        self.font = font
        self.fg = foreground
        self.bg = background

class Text:
    """
.. class:: Text(x, y, font, options, text, palette=None)

    Class to store a text element configuration.
    List of attributes:

        * :attr:`x` x-coordinate top-left, in pixels
        * :attr:`y` y-coordinate top-left, in pixels
        * :attr:`font` font to use ``0-31``
        * :attr:`options` one of ``OPT_CENTERX``, ``OPT_CENTERY``, ``OPT_CENTER``, ``OPT_RIGHTX``, ``OPT_FORMAT``, ``OPT_FILL``
        * :attr:`text` text string 
        * :attr:`palette` :class:`Palette` object instance to set colors
    """
    def __init__(self, x, y, font, options, text, palette=None):
        self.x = x
        self.y = y
        self.font = font
        self.options = options
        self.text = text
        self.palette = palette

class Button(Text):
    """
.. class:: Button(x, y, width, height, font, options, text, palette=None)

    Class to store a text element configuration.
    Inherits all :class:`Text` attributes and adds:

        * :attr:`width` button width in pixels
        * :attr:`height` button height in pixels
    """
    def __init__(self, x, y, width, height, font, options, text, palette=None):
        Text.__init__(self, x, y, font, options, text, palette)
        self.width = width
        self.height = height

OPT_3D = 0
OPT_RGB565 = 0
OPT_CENTER = 1536
OPT_CENTERX = 512
OPT_CENTERY = 1024
OPT_FILL = 8192
OPT_FLASH = 64
OPT_FLAT = 256
OPT_FORMAT = 4096
OPT_FULLSCREEN = 8
OPT_MEDIAFIFO = 16
OPT_MONO = 1
OPT_NOBACK = 4096
OPT_NODL = 2
OPT_NOHANDS = 49152
OPT_NOHM = 16384
OPT_NOPOINTER = 16384
OPT_NOSECS = 32768
OPT_NOTEAR = 4
OPT_NOTICKS = 8192
OPT_OVERLAY = 128
OPT_RIGHTX = 2048
OPT_SIGNED = 256
OPT_SOUND = 32

@native_c("_bt81x_dlstart", [])
def dl_start():
    """
=====================
Co-Processor Commands
=====================

List of available options for the ``options`` command parameter:

    * ``OPT_3D`` 3D effect
    * ``OPT_CENTER`` horizontally and vertically centered style
    * ``OPT_CENTERX`` horizontally-centered style
    * ``OPT_CENTERY`` vertically-centered style
    * ``OPT_FILL`` breaks the text at spaces into multiple lines
    * ``OPT_FLASH`` fetch the data from flash memory
    * ``OPT_FLAT`` no 3D effect
    * ``OPT_FORMAT`` flag of string formatting
    * ``OPT_FULLSCREEN`` zoom the media to fill as much of the screen as possible
    * ``OPT_MEDIAFIFO`` source data from the defined media FIFO
    * ``OPT_MONO`` decodes the source JPEG image to L8 format, i.e., monochrome
    * ``OPT_NOBACK`` no background drawn
    * ``OPT_NODL`` no display list commands generated
    * ``OPT_NOHANDS`` no hands
    * ``OPT_NOHM`` no hour and minute hands
    * ``OPT_NOPOINTER`` no pointer
    * ``OPT_NOSECS`` no second hands
    * ``OPT_NOTEAR`` sync video updates to the display blanking interval, avoiding horizontal *tearing* artifacts
    * ``OPT_NOTICKS`` no ticks
    * ``OPT_OVERLAY`` append the video bitmap to an existing display list
    * ``OPT_RGB565`` decodes the source image to RGB565 format
    * ``OPT_RIGHTX`` right justified style
    * ``OPT_SIGNED`` the number is treated as a 32 bit signed integer
    * ``OPT_SOUND`` decode the audio data

Options can be combined using a bitwise OR.

.. function:: dl_start()

    Starts a new display list.
    """
    pass

@native_c("_bt81x_cocmd_button_text", [])
def _cocmd_button_text(x, y, width, height, font, options, text):
    pass

def _cocmd_button(x, y, width, height, font, options, text):
    _cocmd_button_text(x, y, width, height, font, options, text)

def _cocmd_text(x, y, font, options, text):
    _cocmd_button_text(x, y, 0, 0, font, options, text)

@native_c("_bt81x_color_rgb", [])
def set_font_color(r, g, b):
    """
.. function:: set_font_color(r, g, b)

    :param r: red ``0-255``
    :param g: green ``0-255``
    :param b: blue ``0-255``

    Sets current font color.
    """
    pass

@native_c("_bt81x_color_foreground", [])
def set_foreground(r, g, b):
    """
.. function:: set_foreground(r, g, b)

    :param r: red ``0-255``
    :param g: green ``0-255``
    :param b: blue ``0-255``

    Sets current foreground color.
    """
    pass

@native_c("_bt81x_color_background", [])
def set_background(r, g, b):
    """
.. function:: set_background(r, g, b)

    :param r: red ``0-255``
    :param g: green ``0-255``
    :param b: blue ``0-255``

    Sets current background color.
    """
    pass

def add_text(txt):
    """
.. function:: add_text(txt)

    :param txt: :class:`Text` object instance
    
    Adds a text element to the screen. 

    A call to :func:`set_font_color` is performed if the :py:attr:`Text.palette` attribute is set.
    """
    if txt.palette:
        set_font_color(*txt.palette.font)
    _cocmd_text(txt.x, txt.y, txt.font, txt.options, txt.text)

def add_button(btn):
    """
.. function:: add_button(btn)

    :param btn: :class:`Button` object instance

    Adds a button element to the screen.

    Calls to :func:`set_background`, :func:`set_foreground` and :func:`set_font_color` are performed if the :py:attr:`Text.palette.font` attribute is set.
    """
    if btn.palette:
        if btn.palette.bg:
            set_background(*btn.palette.bg)
        if btn.palette.fg:
            set_foreground(*btn.palette.fg)
        set_font_color(*btn.palette.font)
    _cocmd_button(btn.x, btn.y, btn.width, btn.height, btn.font, btn.options, btn.text)

@native_c("_bt81x_add_keys", [])
def add_keys(x, y, w, h, font, options, s):
    """
.. function:: add_keys(x, y, w, h, font, options, s)

    :param x: x-coordinate top-left, in pixels
    :param y: y-coordinate top-left, in pixels
    :param w: width of the keys
    :param h: height of the keys
    :param font: font used in key label ``0-31``
    :param options: one of ``OPT_3D`` (default), ``OPT_FLAT``, ``OPT_CENTER`` or an ASCII code
    :param s: key labels, one character per key.

    Adds a row of keys to the screen.
    If an ASCII code is specified as option, that key is drawn as *pressed* (in background color with any 3D effect removed).

    The ``TAG`` value is set to the ASCII value of each key, so that key presses can be detected with a callback on that value.
    """
    pass

@native_c("_bt81x_add_clock", [])
def add_clock(x, y, r, options, h, m, s, ms):
    """
.. function:: add_clock(x, y, r, options, h, m, s, ms)

    :param x: x-coordinate top-left, in pixels
    :param y: y-coordinate top-left, in pixels
    :param r: clock radius
    :param options: one of ``OPT_3D`` (default), ``OPT_FLAT``, ``OPT_NOBACK``, ``OPT_NOTICKS``, ``OPT_NOSECS``, ``OPT_NOHANDS``, ``OPT_NOHM``
    :param h: hours
    :param m: minutes
    :param s: seconds
    :param ms: milliseconds

    Adds a clock to the screen.
    """
    pass

@native_c("_bt81x_clear", [])
def clear(color, stencil, tag):
    """
.. function:: clear(color, stencil, tag)

    :param color: clear color ``0-1``
    :param stencil: clear stencil ``0-1``
    :param tag: clear tag ``0-1``

    Clears buffers to default values.
    """
    pass

@native_c("_bt81x_clear_color_rgb", [])
def _clear_color_rgb(r, g, b):
    pass

@native_c("_bt81x_clear_color_a", [])
def _clear_color_a(a):
    pass

def clear_color(rgb=None, a=None):
    """
.. function:: clear_color(rgb=None, a=None)

    :param rgb: tuple for red, green and blue values (``0-255``, ``0-255``, ``0-255``)
    :param a: alpha ``0-255``

    Sets the default color when colors are cleared. The initial value is ``((0, 0, 0), 0)``.
    """
    if rgb is not None:
        _clear_color_rgb(*rgb)
    if a is not None:
        _clear_color_a(a)

@native_c("_bt81x_clear_tag", [])
def clear_tag(default_tag):
    """
.. function:: clear_tag(default_tag)

    :param default_tag: default tag

    Sets the default tag when tag buffer is cleared. The initial value is ``0``.
    """
    pass

SPINNER_CIRCLE = 0
SPINNER_LINE = 1
SPINNER_CLOCK = 2
SPINNER_ORBITING = 3

@native_c("_bt81x_spinner", [])
def spinner(x, y, style, scale):
    """
.. function:: spinner(x, y, style, scale)

    :param x: x-coordinate top-left, in pixels
    :param y: y-coordinate top-left, in pixels
    :param style: spinner style, one of ``SPINNER_CIRCLE``, ``SPINNER_LINE``, ``SPINNER_CLOCK``, ``SPINNER_ORBITING``

    Draws a spinner with a chosen style.
    """
    pass

@native_c("_bt81x_inflate_start", [])
def _inflate_start(ram_ptr):
    pass

@native_c("_bt81x_push_buf", [])
def _push_buf(buf):
    pass

def _push_resource_fifo(resource):
    resource = streams.ResourceStream(resource)
    chunk_size = 512
    chunk_cur = 0

    while True:
        chunk_cur_end = min(chunk_cur + chunk_size, resource.size)
        _push_buf(resource[chunk_cur:chunk_cur_end:1])

        chunk_cur = chunk_cur_end
        if chunk_cur == resource.size:
            break  

def inflate(ram_ptr, resource):
    """
.. function:: inflate(ram_ptr, resource)

    :param ram_ptr: address in RAM_G to inflate the resource to
    :param resource: name of the resource to inflate

    Inflates a Zerynth resource to RAM_G (General purpose graphics RAM, bt81x main memory) for later use.
    The resource should be a valid bt81x image (zlib-compressed)

    :raises PeripheralError: if an error occurs while inflating 
    :raises TimeoutError: if the process takes longer than set timeout (:func:`set_timeout`)
    """
    _inflate_start(ram_ptr)
    _push_resource_fifo(resource)

@native_c("_bt81x_load_image_start", [])
def _load_image_start(ram_ptr, options):
    pass

def load_image(ram_ptr, options, resource):
    """
.. function:: load_image(ram_ptr, options, resource)

    :param ram_ptr: address in RAM_G to load the resource to
    :param options: load options
    :param resource: name of the resource to inflate

    Inflates a Zerynth resource consisting of a PNG image to RAM_G (General purpose graphics RAM, bt81x main memory) for later use.

    :raises PeripheralError: if an error occurs while loading 
    :raises TimeoutError: if the process takes longer than set timeout (:func:`set_timeout`)
    """
    _load_image_start(ram_ptr, options)
    _push_resource_fifo(resource)

@native_c("_bt81x_vertex_format", [])
def vertex_format(fmt):
    """
.. function:: vertex_format(fmt)

    :param fmt: format frac value, one of ``0``, ``1``, ``2``, ``3``, ``4``

    Selects a vertex format for subsequent draw operations.

    Vertex format are useful to specify pixel coordinates beyond the ``0-511`` range.
    """
    pass

@native_c("_bt81x_bitmap_prepare_draw", [])
def _bitmap_prepare_draw(handle, source, img_format, linestride, img_w, img_h, 
                filter, wrapx, wrapy):
    pass

@native_c("_bt81x_bitmap_draw", [])
def _bitmap_draw(vertex_fmt, vtx_x, vtx_y, handle, cell):
    pass

L1 = 1
L2 = 17
L4 = 2
L8 = 3
ARGB2 = 5
RGB332 = 4
ARGB4 = 6
ARGB1555 = 0
RGB565 = 7
PALETTED8 = 16
PALETTED4444 = 15
PALETTED565 = 14

BARGRAPH = 11
TEXT8X8 = 9

NEAREST = 0
BILINEAR = 1

BORDER = 0
REPEAT = 1

class Bitmap:
    """
.. class:: Bitmap(handle, source, layout, size)

    :param handle: a user-defined handle to refer to the bitmap
    :param source: bitmap source in RAM_G
    :param layout: a tuple of ``(bitmap_format, linestride)``
    :param size: a tuple of ``(filtering_mode, x_wrap_mode, y_wrap_mode, bitmap_width, bitmap_height)``

    Class to store a bitmap element and to allow subsequent bitmap draw operations.

    ``linestride`` value represents the amount of memory used for each line of bitmap pixels.
    It depends on selected format and can be usually calculated with the following formula::

        linestride = width * byte/pixel

    Allowed values for ``bitmap_format`` and number of bits/pixel for that format:

        * ``L1`` ``1``
        * ``L2`` ``2`` 
        * ``L4`` ``4``
        * ``L8`` ``8``
        * ``ARGB2`` ``8``
        * ``RGB332`` ``8``
        * ``ARGB4`` ``16``
        * ``ARGB1555`` ``16``
        * ``RGB565`` ``16``
        * ``PALETTED8`` ``8``
        * ``PALETTED4444`` ``8``
        * ``PALETTED565`` ``8``
        * ``BARGRAPH``
        * ``TEXT8X8``

    Allowed values for ``filtering_mode`` are:

        * ``NEAREST``
        * ``BILINEAR``

    Allowed values for ``x_wrap_mode`` and ``y_wrap_mode`` are:

        * ``BORDER``
        * ``REPEAT``

    .. method:: prepare_draw()

            To be called before :meth:`draw`.

    .. method:: draw(vertex, vertex_fmt=None)

        Draws prepared image on screen.
        Can be called multiple times after a single :meth:`prepare_draw`.

        If ``vertex`` tuple has 2 elements the vertex format is set according to ``vertex_fmt`` parameter (:func:`vertex_format`) \
        and ``vertex`` elements are assumed to be the image ``x`` and ``y`` top-left coordinates.

        If ``vertex`` tuple has 4 elements ``vertex_fmt`` parameter is ignored and \
        ``vertex`` elements are assumed to be the image ``x`` and ``y`` top-left coordinates, image handle and cell.

    """
    def __init__(self, handle, source, layout, size):
        self.handle = handle
        self.source = source
        self.layout = layout
        self.size = size

    def prepare_draw(self):
        _bitmap_prepare_draw(self.handle, self.source, self.layout[0], self.layout[1], 
                    self.size[3], self.size[4], self.size[0], self.size[1], self.size[2])

    def draw(self, vertex, vertex_fmt=None):
        if vertex_fmt is not None:
            if len(vertex) != 2: # set format only when calling VERTEX2F
                raise TypeError
            vertex_format(vertex_fmt)

        if len(vertex) == 2:
            _bitmap_draw(1, vertex[0], vertex[1], 0, 0)
        else:
            _bitmap_draw(0, *vertex)


@native_c("_bt81x_end", [])
def end():
    """
.. function:: end()

    Ends drawing a graphics primitive.
    """
    pass

@native_c("_bt81x_display", [])
def display():
    """
.. function:: display()

    Ends a display list.
    """
    pass

@native_c("_bt81x_swap_empty", [])
def swap_and_empty():
    """
.. function:: swap_and_empty()

    Swaps current display list and waits until all commands are executed.

    :raises PeripheralError: if an error occurs while executing commands
    :raises TimeoutError: if the process takes longer than set timeout (:func:`set_timeout`)
    """
    pass

@native_c("_bt81x_waitfifo_timeout", [])
def set_timeout(timeout_millis):
    """
.. function:: set_timeout(timeout_millis)

    :param timeout_millis: timeout in milliseconds

    Sets a timeout for Co-Processor commands. Default timeout value is ``4000``.
    """
    pass

@native_c("_bt81x_tag", [])
def tag(n):
    """
.. function:: tag(n)

    :param n: tag value ``1-255``

    Attaches the tag value to all the following graphics objects drawn on the screen, unless :func:`tag_mask` is used to disable it.
    When the graphics objects attached with the tag value are touched, if calls to :func:`track` and :func:`touch_loop` have been previously issued, a callback function is called.

    The initial tag value is specified by function :func:`clear_tag` and takes effect calling function :func:`clear`.
    """
    pass

@native_c("_bt81x_tag_mask", [])
def tag_mask(mask):
    """
.. function:: tag_mask(mask)

    :param mask: mask value ``0-1``

    If called with value ``0`` the default value of the tag buffer is used for current display list.
    """
    pass

@native_c("_bt81x_track", [])
def track(x, y, width, height, tag):
    """
.. function:: tag_mask(mask)

    :param mask: mask value ``0-1``

    If called with value ``0`` the default value of the tag buffer is used for current display list.
    """
    pass

@native_c("_bt81x_get_reg_tracker", [])
def _get_reg_tracker(tracker_n):
    pass

def _touch_loop(cbks, touch_points):
    while True:
        for cbk in cbks:
            tag = cbk[0]
        for touch_point in range(touch_points):
            regtracker = _get_reg_tracker(touch_point)
            if (tag == -1 and regtracker & 0xff) or (regtracker & 0xff) == tag:
                cbk[1](regtracker & 0xff, regtracker >> 16, touch_point)
        sleep(100)

def touch_loop(cbks, touch_points=1):
    """
.. function:: touch_loop(cbks)

    :param cbks: tuple of tuples of callback and tag value for the callback to be activated on ``((tag_value1, cbk1), (tag_value2, cbk2), ...)``
    :param touch_points: number of multiple touch points (to be supported by used display)

    Starts the touch loop to call set callbacks when touches are detected.
    Each callback function is called passing tag value, tracked value and touch point.

    If a tag value of ``-1`` is specified for a certain callback, that callback is called for every detected tag value.
    """
    thread(_touch_loop, cbks, touch_points)

FLASH_STATUS_INIT = 0
FLASH_STATUS_DETACHED = 1
FLASH_STATUS_BASIC = 2
FLASH_STATUS_FULL = 3

@native_c("_bt81x_flash_status", [])
def flash_status():
    pass

@native_c("_bt81x_flash_attach", [])
def flash_attach():
    pass

@native_c("_bt81x_flash_detach", [])
def flash_detach():
    pass

@native_c("_bt81x_flash_size", [])
def flash_size():
    pass

