#include <stdint.h>
#include <stdarg.h>

#include "printf/printf.h"
#include "tilemap.h"
#include "system.h"
#include "util.h"
#include "input.h"
#include "color.h"

Layer cur_layer;
uint16_t cur_x, cur_y;
uint16_t cur_color;


#if HAS_TC0480SCP
#define SCN TC0480SCP
static inline int width()
{
    switch(cur_layer)
    {
        case FG0: return 64;
        case ROZ: return 64;
        default: return 32;
    }
}
#else
#define SCN TC0100SCN
static inline int width() { return 64; }
#endif

void on_layer(Layer layer)
{
    cur_layer = layer;
}

void pen_color(int color)
{
    cur_color = color;
}

void move_to(int x, int y)
{
    cur_x = x;
    cur_y = y;
}

static void print_string(const char *str)
{
    uint16_t x = cur_x;
    uint16_t y = cur_y;

    uint16_t ofs = ( y * width() ) + x;

    uint16_t attr_color = (0 << 8) | (cur_color & 0xff);

    while(*str)
    {
        if( *str == '\n' )
        {
            y++;
            x = cur_x;
            ofs = (y * width()) + x;
        }
        else
        {
            switch(cur_layer)
            {
                case BG0:
                    SCN->bg0[ofs].attr_color = attr_color;
                    SCN->bg0[ofs].code = *str;
                    break;

                case BG1:
                    SCN->bg1[ofs].attr_color = attr_color;
                    SCN->bg1[ofs].code = *str;
                    break;

#if HAS_TC0480SCP
                case BG2:
                    SCN->bg2[ofs].attr_color = attr_color;
                    SCN->bg2[ofs].code = *str;
                    break;

                case BG3:
                    SCN->bg3[ofs].attr_color = attr_color;
                    SCN->bg3[ofs].code = *str;
                    break;
#endif // HAS_TC0480SCP


                case FG0:
                    SCN->fg0[ofs].attr = cur_color & 0x3f;
                    SCN->fg0[ofs].code = *str;
                    break;

#if HAS_TC0430GRW
                case ROZ:
                    TC0430GRW[ofs] = (cur_color << 14) | *str;
                    break;
#endif //HAS_TC0430GRW
                default:
                    break;
            }
            ofs++;
            x++;
        }
        str++;
    }
    cur_x = x;
    cur_y = y;
}

void print(const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    buf[127] = '\0';
    va_end(args);
    
    print_string(buf);
}

void print_at(int x, int y, const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    buf[127] = '\0';
    va_end(args);
    
    move_to(x, y);
    print_string(buf);
}

void sym_at(int x, int y, uint16_t sym)
{
    static uint16_t attr = 0;
    uint16_t ofs = ( y * width() ) + x;
    uint16_t attr_color = cur_color;

    switch(cur_layer)
    {
        case BG0:
            SCN->bg0[ofs].attr_color = attr_color;
            SCN->bg0[ofs].code = sym;
            break;

        case BG1:
            SCN->bg1[ofs].attr_color = attr_color;
            SCN->bg1[ofs].code = sym;
            break;

#if HAS_TC0480SCP
        case BG2:
            SCN->bg2[ofs].attr_color = attr_color;
            SCN->bg2[ofs].code = sym;
            break;

        case BG3:
            SCN->bg3[ofs].attr_color = attr_color;
            SCN->bg3[ofs].code = sym;
            break;
#endif

        case FG0:
            SCN->fg0[ofs].attr = cur_color;
            SCN->fg0[ofs].code = sym;
            break;
        
        case ROZ:
#if HAS_TC0430GRW
            TC0430GRW[ofs] = (cur_color << 14) | sym;
#endif // HAS_TC0430GRW
            break;

        default:
            break;
    }
}

#if HAS_TC0480SCP
uint32_t *fg0_ptr = NULL;

void fg0_gfx(int ch)
{
    fg0_ptr = SCN->fg0_gfx + (8 * ch);
}

void fg0_row(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
{
    uint32_t gfx;
    gfx  = (x0&0xf) << 16 | (x1&0xf) << 20 | (x2&0xf) << 24 | (x3&0xf) << 28;
    gfx |= (x4&0xf) <<  0 | (x5&0xf) <<  4 | (x6&0xf) <<  8 | (x7&0xf) << 12;
    *fg0_ptr = gfx;
    fg0_ptr++;
}

void fg0_row_2bpp(uint16_t r)
{
    fg0_row(
        ((r >> 7) & 1) | ((r >> 14) & 2),
        ((r >> 6) & 1) | ((r >> 13) & 2),
        ((r >> 5) & 1) | ((r >> 12) & 2),
        ((r >> 4) & 1) | ((r >> 11) & 2),
        ((r >> 3) & 1) | ((r >> 10) & 2),
        ((r >> 2) & 1) | ((r >>  9) & 2),
        ((r >> 1) & 1) | ((r >>  8) & 2),
        ((r >> 0) & 1) | ((r >>  7) & 2)
    );
}

#else
uint16_t *fg0_ptr = NULL;

void fg0_gfx(int ch)
{
    fg0_ptr = SCN->fg0_gfx + (8 * ch);
}

void fg0_row(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
{
    uint16_t gfx;
    gfx  = (x0&1) << 7 | (x1&1) << 6 | (x2&1) << 5 | (x3&1) << 4 | (x4&1) << 3 | (x5&1) << 2 | (x6&1) << 1 | (x7&1) << 0;
    gfx |= (x0&2) << 14 | (x1&2) << 13 | (x2&2) << 12 | (x3&2) << 11 | (x4&2) << 10 | (x5&2) << 9 | (x6&2) << 8 | (x7&2) << 7;
    *fg0_ptr = gfx;
    fg0_ptr++;
}

void fg0_row_2bpp(uint16_t r)
{
    *fg0_ptr = r;
    fg0_ptr++;
}

#endif

extern char _binary_font_chr_start[];
extern char _binary_font_chr_end[];

void reset_screen_config()
{
    bool flip = (input_dsw() & 0x0040) == 0;

#if HAS_TC0480SCP
    TC0480SCP_Ctrl->system_flags = TC0480SCP_SYSTEM_EXT_SYNC | ( flip ? TC0480SCP_SYSTEM_FLIP : 0 );
    
    int16_t base_x = flip ? -122 : 39;
    int16_t base_y = flip ? (-261) : -5;
    TC0480SCP_Ctrl->fg0_y = flip ? -253 : 13;
    TC0480SCP_Ctrl->fg0_x = flip ? 134 : 49;
    TC0480SCP_Ctrl->bg0_y = base_y;
    TC0480SCP_Ctrl->bg0_x = base_x - 00;
    TC0480SCP_Ctrl->bg1_y = base_y;
    TC0480SCP_Ctrl->bg1_x = base_x - 4;
    TC0480SCP_Ctrl->bg2_y = base_y;
    TC0480SCP_Ctrl->bg2_x = base_x - 8;
    TC0480SCP_Ctrl->bg3_y = base_y;
    TC0480SCP_Ctrl->bg3_x = base_x - 12;
    TC0480SCP_Ctrl->bg0_zoom = 0x7f;
    TC0480SCP_Ctrl->bg1_zoom = 0x7f;
    TC0480SCP_Ctrl->bg2_zoom = 0x7f;
    TC0480SCP_Ctrl->bg3_zoom = 0x7f;
    TC0480SCP_Ctrl->bg0_dx = 0;
    TC0480SCP_Ctrl->bg0_dy = 0;
    TC0480SCP_Ctrl->bg1_dx = 0;
    TC0480SCP_Ctrl->bg1_dy = 0;
    TC0480SCP_Ctrl->bg2_dx = 0;
    TC0480SCP_Ctrl->bg2_dy = 0;
    TC0480SCP_Ctrl->bg3_dx = 0;
    TC0480SCP_Ctrl->bg3_dy = 0;
#else
    int16_t base_x;
    int16_t base_y;
    uint16_t system_flags;

    if (flip)
    {
        base_x = 7;
        base_y = 16;
        system_flags = TC0100SCN_SYSTEM_FLIP;
    }
    else
    {
        base_x = 9;
        base_y = 0;
        system_flags = 0;
    }

    TC0100SCN_Ctrl->bg1_y = base_y;
    TC0100SCN_Ctrl->bg1_x = base_x;
    TC0100SCN_Ctrl->fg0_y = base_y;
    TC0100SCN_Ctrl->fg0_x = base_x;
    TC0100SCN_Ctrl->system_flags = system_flags;
#if 0 //GAME_DRIFTOUT
    TC0100SCN_Ctrl->layer_flags = TC0100SCN_LAYER_BG0_DISABLE | TC0100SCN_LAYER_BG1_DISABLE;
#else
    TC0100SCN_Ctrl->layer_flags = 0;
#endif    
    TC0100SCN_Ctrl->bg0_y = base_y;
    TC0100SCN_Ctrl->bg0_x = base_x;
#endif

}

void reset_screen()
{
#if HAS_TC0480SCP
    memset(TC0480SCP, 0, sizeof(TC0480SCP_Layout));
#else
    memset(TC0100SCN, 0, sizeof(TC0100SCN_Layout));
#endif

    *(uint16_t *)0x300006 = 2;

    reset_screen_config();

    fg0_gfx(0x20);
    uint16_t *fgptr = (uint16_t *)_binary_font_chr_start;
    while( fgptr != (uint16_t *)_binary_font_chr_end )
    {
        fg0_row_2bpp(*fgptr);
        fgptr++;
    }

    fg0_gfx(1);
    
    for( int i = 0; i < 8; i++ )
    {
        fg0_row(1,1,1,1,1,1,1,1);
    }
    memset(TC0200OBJ, 0, 0x10000);

    set_default_palette();

#if HAS_TC0360PRI
    tc0360pri_set_obj_prio(5, 5, 7, 9);
    tc0360pri_set_tile_prio(2, 2, 2);
    tc0360pri_set_roz_prio(3, 3, 3, 3);
    tc0360pri_set_roz(0, 0);
#endif

#if HAS_TC0430GRW
    memset(TC0430GRW, 0, 64 * 64 * 2);

    TC0430GRW_Ctrl->origin_x = 0 << 12;
    TC0430GRW_Ctrl->origin_y = 8 << 12;
    TC0430GRW_Ctrl->dxx = 1 << 12;
    TC0430GRW_Ctrl->dxy = 0;
    TC0430GRW_Ctrl->dyx = 0;
    TC0430GRW_Ctrl->dyy = 1 << 12;
#endif
}


