/*
* @Author: lorenzo
* @Date:   2018-10-10 11:32:49
* @Last Modified by:   Lorenzo
* @Last Modified time: 2018-12-06 17:24:22
*/

#include "platform.h"

#define cstringify(bstring) do {                           \
    bstring ## _cstring = gc_malloc(bstring ## _len + 1);  \
    memcpy(bstring ## _cstring, bstring, bstring ## _len); \
    bstring ## _cstring[bstring ## _len] = 0;              \
} while (0)

#define VAR_LIST(...) __VA_ARGS__
#define VAR_DEC(type, ...) type VAR_LIST(__VA_ARGS__)

#define C_NATIVE_PARSE_CALL(name, vars, parse_str, parse_args, call) C_NATIVE(name) { \
    NATIVE_UNWARN(); \
    vars;  \
    if (parse_py_args(parse_str, nargs, args, parse_args) != (sizeof(parse_str) - 1)) \
        return ERR_TYPE_EXC; \
    call; \
    return ERR_OK; \
}

#define C_NATIVE_SIMPLE_WRAP(name, call) C_NATIVE(name) { \
    NATIVE_UNWARN(); \
    call; \
    return ERR_OK; \
}

#define PARSE_PY_PINT(var) \
    if (nargs>0 && (PTYPE(args[0])==PINTEGER || PTYPE(args[0])==PSMALLINT)){ \
        var = INTEGER_VALUE(args[0]);                                        \
        nargs--; args++;                                                     \
    } else                                                                   \
        var = -1


int SPI_DEVICE;
int SPI_SPEED_HZ;
gpio_name GPIO_CS;
gpio_name GPIO_PD;
gpio_name GPIO_INT;

Gpu_Hal_Context_t host, *phost;
zerynth_waitfifo_error_t zerynth_waitfifo_error;
uint32_t zerynth_waitfifo_timeout = 4000;

C_NATIVE(_bt81x_init) {
    phost = &host;

    uint32_t width, height, 
            hcycle, hoffset, hsync0, hsync1, 
            vcycle, voffset, vsync0, vsync1, 
            pclk, swizzle, pclkpol, cspread, dither;

    uint32_t spidrv;
    if (parse_py_args("iiiiiiiiiiiiiiiiiiii", nargs, args, &spidrv, &GPIO_CS, &GPIO_PD, &GPIO_INT, &SPI_SPEED_HZ,
                                                        &width, &height, 
                                                        &hcycle, &hoffset, &hsync0, &hsync1, 
                                                        &vcycle, &voffset, &vsync0, &vsync1, 
                                                        &pclk, &swizzle, &pclkpol, &cspread, &dither) != 20)
        return ERR_TYPE_EXC;

    SPI_DEVICE = spidrv & 0xff;

    Gpu_HalInit_t halinit;
    uint8_t chipid;

    Gpu_Hal_Init(&halinit);
    Gpu_Hal_Open(phost);

    Gpu_Hal_Powercycle(phost,TRUE);

    /* FT81x will be in SPI Single channel after POR
    If we are here with FT4222 in multi channel, then
    an explicit switch to single channel is essential
    */
    Gpu_Hal_SetSPI(phost, GPU_SPI_SINGLE_CHANNEL, GPU_SPI_ONEDUMMY);

    /* access address 0 to wake up the FT800 */
    Gpu_HostCommand(phost,GPU_ACTIVE_M);
    Gpu_Hal_Sleep(300);

    /* read Register ID to check if FT800 is ready */
    chipid = Gpu_Hal_Rd8(phost, REG_ID);
    while(chipid != 0x7C)
      {
        chipid = Gpu_Hal_Rd8(phost, REG_ID);
        Gpu_Hal_Sleep(100);
      }

    /* configuration of LCD display */
    Gpu_Hal_Wr16(phost, REG_HCYCLE, hcycle);
    Gpu_Hal_Wr16(phost, REG_HOFFSET, hoffset);
    Gpu_Hal_Wr16(phost, REG_HSYNC0, hsync0);
    Gpu_Hal_Wr16(phost, REG_HSYNC1, hsync1);
    Gpu_Hal_Wr16(phost, REG_VCYCLE, vcycle);
    Gpu_Hal_Wr16(phost, REG_VOFFSET, voffset);
    Gpu_Hal_Wr16(phost, REG_VSYNC0, vsync0);
    Gpu_Hal_Wr16(phost, REG_VSYNC1, vsync1);
    Gpu_Hal_Wr8(phost, REG_SWIZZLE, swizzle);
    Gpu_Hal_Wr8(phost, REG_PCLK_POL, pclkpol);
    Gpu_Hal_Wr16(phost, REG_HSIZE, width);
    Gpu_Hal_Wr16(phost, REG_VSIZE, height);
    Gpu_Hal_Wr16(phost, REG_CSPREAD, cspread);
    Gpu_Hal_Wr16(phost, REG_DITHER, dither);

    /* GPIO configuration */
    Gpu_Hal_Wr16(phost, REG_GPIOX_DIR, 0xffff);
    Gpu_Hal_Wr16(phost, REG_GPIOX, 0xffff);

    Gpu_ClearScreen(phost);

    /* after this display is visible on the LCD */
    Gpu_Hal_Wr8(phost, REG_PCLK, pclk);

    /* enable global interrupts */
    // Gpu_Hal_Wr16(phost, REG_INT_EN, 1);

    phost->cmd_fifo_wp = Gpu_Hal_Rd16(phost,REG_CMD_WRITE);

    return ERR_OK;
}

static inline int fifo_empty_with_errors() {
    Gpu_Hal_WaitCmdfifo_empty(phost);

    switch (zerynth_waitfifo_error) {
        case ZERYNTH_WAITFIFO_ERROR_FAULT: {
            uint8_t* report_buffer = gc_malloc(128);
            Gpu_Hal_RdMem(phost, RAM_ERR_REPORT, report_buffer, 128);

            vbl_printf_stdout("%s\n", report_buffer);
            gc_free(report_buffer);

            return ERR_PERIPHERAL_ERROR_EXC;
        }
        case ZERYNTH_WAITFIFO_ERROR_TIMEOUT: {
            vbl_printf_stdout("read %x write %x\n", 
                        Gpu_Hal_Rd16(phost,REG_CMD_READ), Gpu_Hal_Rd16(phost,REG_CMD_WRITE));
            return ERR_TIMEOUT_EXC;
        }
        default:
            return ERR_OK;
    }
}

C_NATIVE(_bt81x_cocmd_button_text) {
    NATIVE_UNWARN();

    uint32_t x, y, width, height, font, options, text_len;
    uint8_t *text, *text_cstring;

    if (parse_py_args("iiiiiis", nargs, args, &x, &y, &width, &height, &font, &options, &text, &text_len) != 7)
        return ERR_TYPE_EXC;

    cstringify(text);
    if (!width) {
        Gpu_CoCmd_Text(phost, x, y, font, options, text_cstring);
    }
    else {
        Gpu_CoCmd_Button(phost, x, y, width, height, font, options, text_cstring);
    }
    gc_free(text_cstring);

    return ERR_OK;
}

C_NATIVE_PARSE_CALL(_bt81x_add_keys,
    VAR_DEC(uint32_t, x, y, width, height, font, options, text_len);
    VAR_DEC(uint8_t, *text, *text_cstring),
    "iiiiiis", VAR_LIST(&x, &y, &width, &height, &font, &options, &text, &text_len),
    cstringify(text);
    Gpu_CoCmd_Keys(phost, x, y, width, height, font, options, text_cstring);
    gc_free(text_cstring)
)

C_NATIVE_PARSE_CALL(_bt81x_add_clock,
    VAR_DEC(uint32_t, x, y, r, options, h, m, s, ms),
    "iiiiiiii", VAR_LIST(&x, &y, &r, &options, &h, &m, &s, &ms),
    Gpu_CoCmd_Clock(phost, x, y, r, options, h, m, s, ms);
)

C_NATIVE_PARSE_CALL(_bt81x_color_rgb, 
    VAR_DEC(uint32_t, red, green, blue),
    "iii", VAR_LIST(&red, &green, &blue),
    Gpu_Hal_WrCmd32(phost, COLOR_RGB(red, green, blue))
)

C_NATIVE_PARSE_CALL(_bt81x_color_foreground, 
    VAR_DEC(uint32_t, red, green, blue),
    "iii", VAR_LIST(&red, &green, &blue),
    Gpu_CoCmd_FgColor(phost, COLOR_RGB(red, green, blue))
)

C_NATIVE_PARSE_CALL(_bt81x_color_background, 
    VAR_DEC(uint32_t, red, green, blue),
    "iii", VAR_LIST(&red, &green, &blue),
    Gpu_CoCmd_BgColor(phost, COLOR_RGB(red, green, blue))
)

C_NATIVE_PARSE_CALL(_bt81x_tag, 
    VAR_DEC(uint32_t, tag),
    "i", VAR_LIST(&tag),
    Gpu_Hal_WrCmd32(phost, TAG(tag))
)

C_NATIVE_PARSE_CALL(_bt81x_tag_mask, 
    VAR_DEC(uint32_t, mask),
    "i", VAR_LIST(&mask),
    Gpu_Hal_WrCmd32(phost, TAG_MASK(mask))
)

C_NATIVE_PARSE_CALL(_bt81x_clear, 
    VAR_DEC(uint32_t, color, stencil, tag),
    "iii", VAR_LIST(&color, &stencil, &tag),
    Gpu_Hal_WrCmd32(phost, CLEAR(color, stencil, tag))
)

C_NATIVE_PARSE_CALL(_bt81x_clear_color_rgb, 
    VAR_DEC(uint32_t, red, green, blue),
    "iii", VAR_LIST(&red, &green, &blue),
    Gpu_Hal_WrCmd32(phost, CLEAR_COLOR_RGB(red, green, blue))
)

C_NATIVE_PARSE_CALL(_bt81x_clear_color_a, 
    VAR_DEC(uint32_t, alpha),
    "i", VAR_LIST(&alpha),
    Gpu_Hal_WrCmd32(phost, CLEAR_COLOR_A(alpha))
)

C_NATIVE_PARSE_CALL(_bt81x_clear_tag, 
    VAR_DEC(uint32_t, default_tag),
    "i", VAR_LIST(&default_tag),
    Gpu_Hal_WrCmd32(phost, CLEAR_TAG(default_tag))
)

C_NATIVE_PARSE_CALL(_bt81x_track, 
    VAR_DEC(uint32_t, x, y, width, height, tag),
    "iiiii", VAR_LIST(&x, &y, &width, &height, &tag),
    Gpu_CoCmd_Track(phost, x, y, width, height, tag)
)

C_NATIVE_PARSE_CALL(_bt81x_spinner, 
    VAR_DEC(uint32_t, x, y, style, scale),
    "iiii", VAR_LIST(&x, &y, &style, &scale),
    Gpu_CoCmd_Spinner(phost, x, y, style, scale)
)

C_NATIVE_PARSE_CALL(_bt81x_vertex_format, 
    VAR_DEC(uint32_t, fmt),
    "i", VAR_LIST(&fmt),
    Gpu_Hal_WrCmd32(phost, VERTEX_FORMAT(fmt))
)

C_NATIVE_SIMPLE_WRAP(_bt81x_display, Gpu_Hal_WrCmd32(phost, DISPLAY()))
C_NATIVE_SIMPLE_WRAP(_bt81x_dlstart, Gpu_CoCmd_Dlstart(phost))

C_NATIVE(_bt81x_swap_empty) {
    NATIVE_UNWARN();

    Gpu_CoCmd_Swap(phost);
    return fifo_empty_with_errors();
}

C_NATIVE(_bt81x_get_reg_tracker) {
    NATIVE_UNWARN();

    uint32_t tracker_n;
    if (parse_py_args("i", nargs, args, &tracker_n) != 1)
        return ERR_TYPE_EXC;

    *res = PSMALLINT_NEW(Gpu_Hal_Rd32(phost, REG_TRACKER + 4*tracker_n));
    return ERR_OK;
}

C_NATIVE_PARSE_CALL(_bt81x_inflate_start, 
    VAR_DEC(uint32_t, ram_ptr),
    "i", VAR_LIST(&ram_ptr),
    Gpu_CoCmd_Inflate(phost, ram_ptr)
)

C_NATIVE_PARSE_CALL(_bt81x_load_image_start, 
    VAR_DEC(uint32_t, ram_ptr, options),
    "ii", VAR_LIST(&ram_ptr, &options),
    Gpu_CoCmd_LoadImage(phost, ram_ptr, options)
)

C_NATIVE_PARSE_CALL(_bt81x_waitfifo_timeout, 
    VAR_DEC(uint32_t, timeout_millis),
    "i", VAR_LIST(&timeout_millis),
    zerynth_waitfifo_timeout = timeout_millis
)

C_NATIVE(_bt81x_push_buf) {
    NATIVE_UNWARN();

    uint8_t *buf;
    uint32_t buf_len;

    if (parse_py_args("s", nargs, args, &buf, &buf_len) != 1)
        return ERR_TYPE_EXC;

    Gpu_Hal_WrCmdBuf(phost, buf, buf_len);
    return fifo_empty_with_errors();
}

C_NATIVE(_bt81x_bitmap_prepare_draw) {
    NATIVE_UNWARN();

    uint32_t handle, source, img_format, linestride,
             img_w, img_h, filter, wrapx, wrapy;

    if (parse_py_args("iiiiiiiii", nargs, args, &handle, &source, &img_format, &linestride,
                                                &img_w, &img_h, &filter, &wrapx, &wrapy) != 9)
        return ERR_TYPE_EXC;

    Gpu_Hal_WrCmd32(phost, BITMAP_HANDLE(handle));
    Gpu_Hal_WrCmd32(phost, BEGIN(BITMAPS));

    Gpu_Hal_WrCmd32(phost, BITMAP_SOURCE(source));
    Gpu_Hal_WrCmd32(phost, BITMAP_LAYOUT(img_format, linestride, img_h));
    Gpu_Hal_WrCmd32(phost, BITMAP_SIZE(filter, wrapx, wrapy, img_w, img_h));
    Gpu_Hal_WrCmd32(phost, BITMAP_LAYOUT_H(linestride >> 10, img_h >> 10));
    Gpu_Hal_WrCmd32(phost, BITMAP_SIZE_H(img_w >> 9, img_h >> 9));

    return ERR_OK;
}

C_NATIVE(_bt81x_bitmap_draw) {
    NATIVE_UNWARN();

    uint32_t vertex_format, vtx_x, vtx_y, handle, cell;

    if (parse_py_args("iiiii", nargs, args, &vertex_format, &vtx_x, &vtx_y, &handle, &cell) != 5)
        return ERR_TYPE_EXC;  

    if (vertex_format) {
        Gpu_Hal_WrCmd32(phost, VERTEX2F(vtx_x, vtx_y));
    }
    else {
        Gpu_Hal_WrCmd32(phost, VERTEX2II(vtx_x, vtx_y, handle, cell));
    }

    return ERR_OK;   
}

C_NATIVE_SIMPLE_WRAP(_bt81x_end, Gpu_Hal_WrCmd32(phost, END()))

C_NATIVE_SIMPLE_WRAP(_bt81x_flash_status, *res = PSMALLINT_NEW(Gpu_Hal_Rd8(phost, REG_FLASH_STATUS)))
C_NATIVE_SIMPLE_WRAP(_bt81x_flash_attach, Gpu_CoCmd_FlashAttach(phost))
C_NATIVE_SIMPLE_WRAP(_bt81x_flash_detach, Gpu_CoCmd_FlashDetach(phost))
C_NATIVE_SIMPLE_WRAP(_bt81x_flash_size, *res = PSMALLINT_NEW(Gpu_Hal_Rd32(phost, REG_FLASH_SIZE)))
