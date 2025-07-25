#include <stdint.h>
#include <stdarg.h>

#include "printf/printf.h"
#include "tilemap.h"
#include "system.h"

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

