.. module:: bt81x

*************
BT81x library
*************

This module exports classes and functions to handle Bridgetek BT81x family of Embedded Video Engines.

    
.. class:: DisplayConf(width, height,                     hcycle, hoffset, hsync0, hsync1,                     vcycle, voffset, vsync0, vsync1,                     pclk, swizzle, pclkpol, cspread, dither,                     description)

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

    
.. function:: init(spi, cs, pd, int, dc=None, spi_speed=3000000)

    :param spi: spi driver (``SPI0``, ``SPI1``, ``...``)
    :param cs: chip select pin
    :param pd: pd pin
    :param int: interrupt pin
    :param dc: display configuration as a :class:`DisplayConf` instance
    :param spi_speed: spi speed in Hertz

    Initializes the chip.

    When ``dc`` parameter is not specified ``display_conf`` global variable is used.
    
.. class:: Palette(font, foreground, background)

    Class to store a color palette for font, foreground and background.
    List of attributes:

        * :attr:`font` tuple of rgb values ``(r,g,b)``
        * :attr:`foreground` tuple of rgb values ``(r,g,b)``
        * :attr:`background` tuple of rgb values ``(r,g,b)``
    
.. class:: Text(x, y, font, options, text, palette=None)

    Class to store a text element configuration.
    List of attributes:

        * :attr:`x` x-coordinate top-left, in pixels
        * :attr:`y` y-coordinate top-left, in pixels
        * :attr:`font` font to use ``0-31``
        * :attr:`options` one of ``OPT_CENTERX``, ``OPT_CENTERY``, ``OPT_CENTER``, ``OPT_RIGHTX``, ``OPT_FORMAT``, ``OPT_FILL``
        * :attr:`text` text string 
        * :attr:`palette` :class:`Palette` object instance to set colors
    
.. class:: Button(x, y, width, height, font, options, text, palette=None)

    Class to store a text element configuration.
    Inherits all :class:`Text` attributes and adds:

        * :attr:`width` button width in pixels
        * :attr:`height` button height in pixels
    
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
    
.. function:: set_font_color(r, g, b)

    :param r: red ``0-255``
    :param g: green ``0-255``
    :param b: blue ``0-255``

    Sets current font color.
    
.. function:: set_foreground(r, g, b)

    :param r: red ``0-255``
    :param g: green ``0-255``
    :param b: blue ``0-255``

    Sets current foreground color.
    
.. function:: set_background(r, g, b)

    :param r: red ``0-255``
    :param g: green ``0-255``
    :param b: blue ``0-255``

    Sets current background color.
    
.. function:: add_text(txt)

    :param txt: :class:`Text` object instance
    
    Adds a text element to the screen. 

    A call to :func:`set_font_color` is performed if the :py:attr:`Text.palette` attribute is set.
    
.. function:: add_button(btn)

    :param btn: :class:`Button` object instance

    Adds a button element to the screen.

    Calls to :func:`set_background`, :func:`set_foreground` and :func:`set_font_color` are performed if the :py:attr:`Text.palette.font` attribute is set.
    
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
    
.. function:: clear(color, stencil, tag)

    :param color: clear color ``0-1``
    :param stencil: clear stencil ``0-1``
    :param tag: clear tag ``0-1``

    Clears buffers to default values.
    
.. function:: clear_color(rgb=None, a=None)

    :param rgb: tuple for red, green and blue values (``0-255``, ``0-255``, ``0-255``)
    :param a: alpha ``0-255``

    Sets the default color when colors are cleared. The initial value is ``((0, 0, 0), 0)``.
    
.. function:: clear_tag(default_tag)

    :param default_tag: default tag

    Sets the default tag when tag buffer is cleared. The initial value is ``0``.
    
.. function:: spinner(x, y, style, scale)

    :param x: x-coordinate top-left, in pixels
    :param y: y-coordinate top-left, in pixels
    :param style: spinner style, one of ``SPINNER_CIRCLE``, ``SPINNER_LINE``, ``SPINNER_CLOCK``, ``SPINNER_ORBITING``

    Draws a spinner with a chosen style.
    
.. function:: inflate(ram_ptr, resource)

    :param ram_ptr: address in RAM_G to inflate the resource to
    :param resource: name of the resource to inflate

    Inflates a Zerynth resource to RAM_G (General purpose graphics RAM, bt81x main memory) for later use.
    The resource should be a valid bt81x image (zlib-compressed)

    :raises PeripheralError: if an error occurs while inflating 
    :raises TimeoutError: if the process takes longer than set timeout (:func:`set_timeout`)
    
.. function:: load_image(ram_ptr, options, resource)

    :param ram_ptr: address in RAM_G to load the resource to
    :param options: load options
    :param resource: name of the resource to inflate

    Inflates a Zerynth resource consisting of a PNG image to RAM_G (General purpose graphics RAM, bt81x main memory) for later use.

    :raises PeripheralError: if an error occurs while loading 
    :raises TimeoutError: if the process takes longer than set timeout (:func:`set_timeout`)
    
.. function:: vertex_format(fmt)

    :param fmt: format frac value, one of ``0``, ``1``, ``2``, ``3``, ``4``

    Selects a vertex format for subsequent draw operations.

    Vertex format are useful to specify pixel coordinates beyond the ``0-511`` range.
    
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

        If ``vertex`` tuple has 2 elements the vertex format is set according to ``vertex_fmt`` parameter (:func:`vertex_format`)         and ``vertex`` elements are assumed to be the image ``x`` and ``y`` top-left coordinates.

        If ``vertex`` tuple has 4 elements ``vertex_fmt`` parameter is ignored and         ``vertex`` elements are assumed to be the image ``x`` and ``y`` top-left coordinates, image handle and cell.

    
.. function:: end()

    Ends drawing a graphics primitive.
    
.. function:: display()

    Ends a display list.
    
.. function:: swap_and_empty()

    Swaps current display list and waits until all commands are executed.

    :raises PeripheralError: if an error occurs while executing commands
    :raises TimeoutError: if the process takes longer than set timeout (:func:`set_timeout`)
    
.. function:: set_timeout(timeout_millis)

    :param timeout_millis: timeout in milliseconds

    Sets a timeout for Co-Processor commands. Default timeout value is ``4000``.
    
.. function:: tag(n)

    :param n: tag value ``1-255``

    Attaches the tag value to all the following graphics objects drawn on the screen, unless :func:`tag_mask` is used to disable it.
    When the graphics objects attached with the tag value are touched, if calls to :func:`track` and :func:`touch_loop` have been previously issued, a callback function is called.

    The initial tag value is specified by function :func:`clear_tag` and takes effect calling function :func:`clear`.
    
.. function:: tag_mask(mask)

    :param mask: mask value ``0-1``

    If called with value ``0`` the default value of the tag buffer is used for current display list.
    
.. function:: tag_mask(mask)

    :param mask: mask value ``0-1``

    If called with value ``0`` the default value of the tag buffer is used for current display list.
    
.. function:: touch_loop(cbks)

    :param cbks: tuple of tuples of callback and tag value for the callback to be activated on ``((tag_value1, cbk1), (tag_value2, cbk2), ...)``
    :param touch_points: number of multiple touch points (to be supported by used display)

    Starts the touch loop to call set callbacks when touches are detected.
    Each callback function is called passing tag value, tracked value and touch point.

    If a tag value of ``-1`` is specified for a certain callback, that callback is called for every detected tag value.
    
