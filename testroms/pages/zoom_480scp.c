#include "../system.h"

#if HAS_TC0480SCP

#include "../page.h"
#include "../util.h"
#include "../input.h"
#include "../tilemap.h"
#include "../obj_test.h"

static void init_zoom_480scp()
{
    reset_screen();

    tc0360pri_set_tile_prio2(4, 4, 4, 4);
    tc0360pri_set_roz_prio(5, 5, 5, 5);
    tc0360pri_set_roz(0, 0);
    

    on_layer(BG0);
    pen_color(8);
    for( int x = 0; x < 8; x++ )
    {
        for( int y = 0; y < 8; y++ )
        {
            sym_at(4+x,4+y,2);
        }
    }
    on_layer(FG0);
    for( int y = 0; y < 16; y++ )
    {
        pen_color(0x18 + (y&1));
        sym_at(23, 7 + y, 1);
        sym_at(7 + y, 23, 1);
    }

    on_layer(BG1);
    pen_color(0x1a);
    sym_at(6,5,1);
    on_layer(BG2);
    pen_color(0x1c);
    sym_at(6,5,1);
    TC0480SCP_Ctrl->bg1_zoom = 0x00be;
    TC0480SCP_Ctrl->bg1_y = 29;
}

typedef struct
{
    uint32_t modified;
    uint32_t zoomy;
    uint32_t dy;
    uint32_t y;
    uint32_t zoomx;
    uint32_t dx;
    uint32_t x;
} SimDebug;

volatile SimDebug *DEBUG = (volatile SimDebug *)0x080000;

// Zoom Notes
// Y
// dy has 7-bit precision
// Zoom 0 = 50%, 7F = 100% BF = 200%
//      SIM  =  HW
// BF Y 0x35     - 0x2D    = 8
// 7f Y 0x0b     - 0x0b
// 00 Y 0xffb6   - 0xffc8  = -18
static void update_zoom_480scp()
{
    static uint8_t sel = 0;
    
    static uint8_t zoomy = 0x7f;
    static uint16_t y = 0;
    static uint8_t dy = 0;
    static uint8_t zoomx = 0x00;
    static uint16_t x = 0;
    static uint8_t dx = 0;


    static uint32_t dbg_modifed = 0;

    if (dbg_modifed == 0)
        dbg_modifed = DEBUG->modified;

    pen_color(8);
    on_layer(FG0);
    print_at(2,3,"VBLANK: %04X", vblank_count);

    print_at(29, 8,  " Y:%04X ", y);
    print_at(29, 9,  " ZY: %02X ", zoomy);
    print_at(29, 10, " DY: %02X ", dy);
    print_at(29, 11, " X:%04X ", x);
    print_at(29, 12, " ZX: %02X ", zoomx);
    print_at(29, 13, " DX: %02X ", dx);

    for( int y = 0; y < 6; y++ )
    {
        sym_at(28, 8 + y, sel == y ? '>' : ' ');
        sym_at(37, 8 + y, sel == y ? '<' : ' ');
    }
    
    wait_vblank();
 
    TC0480SCP_Ctrl->bg1_zoom = (zoomy & 0xff) | ( zoomx << 8 );
    TC0480SCP_Ctrl->bg1_y = y;
    TC0480SCP_Ctrl->bg1_dy = dy;
    TC0480SCP_Ctrl->bg1_x = x;
    TC0480SCP_Ctrl->bg1_dx = dx;
    TC0480SCP_Ctrl->bg2_zoom = (zoomy & 0xff) | ( zoomx << 8 );
    TC0480SCP_Ctrl->bg2_y = y;
    TC0480SCP_Ctrl->bg2_dy = dy;
    TC0480SCP_Ctrl->bg2_x = x - 4;
    TC0480SCP_Ctrl->bg2_dx = dx;



    int dir = 0;
    if (input_pressed(DOWN)) sel++;
    if (input_pressed(UP)) sel--;
    if (input_pressed(LEFT)) dir = -1;
    if (input_pressed(RIGHT)) dir = 1;
    if (input_down(BTN1)) dir *= 8;

    switch(sel)
    {
        case 0: y += dir; break;
        case 1: zoomy += dir; break;
        case 2: dy += dir; break;
        case 3: x += dir; break;
        case 4: zoomx += dir; break;
        case 5: dx += dir; break;
        case 6: sel = 0; break;
        default: sel = 5; break;
    }

    if (DEBUG->modified != dbg_modifed)
    {
        dbg_modifed = DEBUG->modified;
        y = DEBUG->y;
        zoomy = DEBUG->zoomy;
        dy = DEBUG->dy;
        x = DEBUG->x;
        zoomx = DEBUG->zoomx;
        dx = DEBUG->dx;
    }
}

PAGE_REGISTER(zoom_480scp, init_zoom_480scp, update_zoom_480scp, NULL);

#endif

