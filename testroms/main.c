#include <stdint.h>
#include <stdbool.h>
#include "printf/printf.h"

#include "tc0100scn.h"
#include "tc0200obj.h"
#include "tc0480scp.h"
#include "util.h"
#include "interrupts.h"
#include "tilemap.h"
#include "input.h"
#include "system.h"
#include "obj_test.h"
#include "color.h"
#include "crc16.h"

volatile uint32_t vblank_count = 0;
volatile uint32_t dma_count = 0;

void level5_handler()
{
    vblank_count++;
#if HAS_TC0360PRI
    tc0360pri_vblank();
#endif

#if HAS_TC0220IOC
    TC0220IOC->watchdog = 0;
#endif

#if GAME_DEADCONX
    *(volatile uint16_t *)0x800000 = 0;
#endif
}

void level6_handler()
{
    dma_count++;
}

void illegal_instruction_handler()
{
    disable_interrupts();

    on_layer(FG0);
    pen_color(1);
    print_at(10,10,"ILLEGAL INSTRUCTION");

    while(true) {};
}

uint32_t wait_vblank()
{
    uint32_t current = vblank_count;
    uint32_t count;
    while( current == vblank_count )
    {
        count++;
    }
    return count;
}

void wait_dma()
{
    uint32_t current = dma_count;
    while( current == dma_count )
    {
    }
}


uint8_t sine_wave[256] =
{
    0x80, 0x83, 0x86, 0x89, 0x8C, 0x90, 0x93, 0x96,
    0x99, 0x9C, 0x9F, 0xA2, 0xA5, 0xA8, 0xAB, 0xAE,
    0xB1, 0xB3, 0xB6, 0xB9, 0xBC, 0xBF, 0xC1, 0xC4,
    0xC7, 0xC9, 0xCC, 0xCE, 0xD1, 0xD3, 0xD5, 0xD8,
    0xDA, 0xDC, 0xDE, 0xE0, 0xE2, 0xE4, 0xE6, 0xE8,
    0xEA, 0xEB, 0xED, 0xEF, 0xF0, 0xF1, 0xF3, 0xF4,
    0xF5, 0xF6, 0xF8, 0xF9, 0xFA, 0xFA, 0xFB, 0xFC,
    0xFD, 0xFD, 0xFE, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFE, 0xFD,
    0xFD, 0xFC, 0xFB, 0xFA, 0xFA, 0xF9, 0xF8, 0xF6,
    0xF5, 0xF4, 0xF3, 0xF1, 0xF0, 0xEF, 0xED, 0xEB,
    0xEA, 0xE8, 0xE6, 0xE4, 0xE2, 0xE0, 0xDE, 0xDC,
    0xDA, 0xD8, 0xD5, 0xD3, 0xD1, 0xCE, 0xCC, 0xC9,
    0xC7, 0xC4, 0xC1, 0xBF, 0xBC, 0xB9, 0xB6, 0xB3,
    0xB1, 0xAE, 0xAB, 0xA8, 0xA5, 0xA2, 0x9F, 0x9C,
    0x99, 0x96, 0x93, 0x90, 0x8C, 0x89, 0x86, 0x83,
    0x80, 0x7D, 0x7A, 0x77, 0x74, 0x70, 0x6D, 0x6A,
    0x67, 0x64, 0x61, 0x5E, 0x5B, 0x58, 0x55, 0x52,
    0x4F, 0x4D, 0x4A, 0x47, 0x44, 0x41, 0x3F, 0x3C,
    0x39, 0x37, 0x34, 0x32, 0x2F, 0x2D, 0x2B, 0x28,
    0x26, 0x24, 0x22, 0x20, 0x1E, 0x1C, 0x1A, 0x18,
    0x16, 0x15, 0x13, 0x11, 0x10, 0x0F, 0x0D, 0x0C,
    0x0B, 0x0A, 0x08, 0x07, 0x06, 0x06, 0x05, 0x04,
    0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x03,
    0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x0A,
    0x0B, 0x0C, 0x0D, 0x0F, 0x10, 0x11, 0x13, 0x15,
    0x16, 0x18, 0x1A, 0x1C, 0x1E, 0x20, 0x22, 0x24,
    0x26, 0x28, 0x2B, 0x2D, 0x2F, 0x32, 0x34, 0x37,
    0x39, 0x3C, 0x3F, 0x41, 0x44, 0x47, 0x4A, 0x4D,
    0x4F, 0x52, 0x55, 0x58, 0x5B, 0x5E, 0x61, 0x64,
    0x67, 0x6A, 0x6D, 0x70, 0x74, 0x77, 0x7A, 0x7D
};

extern char _binary_font_chr_start[];
extern char _binary_font_chr_end[];

#define NUM_SCREENS 14

static uint32_t frame_count;

void reset_screen()
{
#if HAS_TC0480SCP
    memset(TC0480SCP, 0, sizeof(TC0480SCP_Layout));
#else
    memset(TC0100SCN, 0, sizeof(TC0100SCN_Layout));
#endif

    *(uint16_t *)0x300006 = 2;

#if HAS_TC0480SCP
    int16_t base_x = 0x31;
    int16_t base_y = 13;
    TC0480SCP_Ctrl->fg0_y = base_y;
    TC0480SCP_Ctrl->fg0_x = base_x;
    TC0480SCP_Ctrl->bg0_y = 8 - base_y;
    TC0480SCP_Ctrl->bg0_x = base_x - 10;
    TC0480SCP_Ctrl->bg1_y = 8 - base_y;
    TC0480SCP_Ctrl->bg1_x = base_x - 14;
    TC0480SCP_Ctrl->bg2_y = 8 - base_y;
    TC0480SCP_Ctrl->bg2_x = base_x - 18;
    TC0480SCP_Ctrl->bg3_y = 8 - base_y;
    TC0480SCP_Ctrl->bg3_x = base_x - 22;
    TC0480SCP_Ctrl->system_flags = TC0480SCP_SYSTEM_EXT_SYNC;
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

    bool flip = 0; //(input_dsw() & 0x02) == 0;

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
    memsetw(TC0200OBJ, 0, 0x8000);

    set_default_palette();

#if HAS_TC0360PRI
    tc0360pri_set_obj_prio(5, 5, 7, 9);
    tc0360pri_set_tile_prio(2, 2, 2);
    tc0360pri_set_roz_prio(3, 3, 3, 3);
    tc0360pri_set_roz(0, 0);
#endif

#if HAS_TC0430GRW
    memsetw(TC0430GRW, 0, 64 * 64);

    TC0430GRW_Ctrl->origin_x = 0 << 12;
    TC0430GRW_Ctrl->origin_y = 8 << 12;
    TC0430GRW_Ctrl->dxx = 1 << 12;
    TC0430GRW_Ctrl->dxy = 0;
    TC0430GRW_Ctrl->dyx = 0;
    TC0430GRW_Ctrl->dyy = 1 << 12;
#endif
}

#if GAME_DRIFTOUT
#define SYM_BOX 0x20
#define SYM_SQUARE 0x20
#else
#define SYM_BOX 0x1
#define SYM_SQUARE 0x1b
#endif

void init_scn_general()
{
    reset_screen();

    frame_count = 0;

    // Max extent corner boundaries
    on_layer(BG0); pen_color(0);
    sym_at(1, 1, SYM_BOX);
    sym_at(1, 28, SYM_BOX);
    sym_at(40, 1, SYM_BOX);
    sym_at(40, 28, SYM_BOX);

    // Should not be visible
    pen_color(1);
    sym_at(0, 1, SYM_SQUARE);
    sym_at(1, 0, SYM_SQUARE);
    sym_at(0, 28, SYM_SQUARE);
    sym_at(1, 29, SYM_SQUARE);
    sym_at(40, 0, SYM_SQUARE);
    sym_at(41, 1, SYM_SQUARE);
    sym_at(40, 29, SYM_SQUARE);
    sym_at(41, 28, SYM_SQUARE);

    pen_color(6);
    print_at(4, 5, "LAYER BG0");
    on_layer(BG1);
    pen_color(3);
    print_at(4, 6, "LAYER BG1");

    on_layer(FG0);
    pen_color(0);
    print_at(4, 9, "The quick brown fox\njumps over the lazy dog.\n0123456789?/=-+*");


    on_layer(BG0);
    pen_color(9);
    sym_at(8, 14, 0x12);
    sym_at(8, 15, 0x12);
    sym_at(8, 16, 0x12);
    //print_at(8, 14, "ROW\nSCROLL\nBG0");
    print_at(8, 18, "-16 ROW\n-8 SCROLL\n 0 BG0\n 8 FIXED");

    on_layer(BG1);
    pen_color(2);
    print_at(19, 14, " COLSCROLL\n1222222222\n9012345678\n\n\nROWCOL\nSCROLL");

    on_layer(ROZ);
    pen_color(0);
    sym_at(2, 1, 0x67);
    sym_at(1, 2, 0x67);
    sym_at(40, 27, 0x67);
    sym_at(39, 28, 0x67);
}

void update_scn_general()
{
    wait_vblank();


    for( int y = 0; y < 24; y++ )
    {
//        TC0100SCN->bg0_rowscroll[14 * 8 + y] = sine_wave[(frame_count*2+(y*4)) & 0xff] >> 4;
    }

    for( int y = 0; y < 32; y++ )
    {
        //TC0100SCN->bg0_rowscroll[18 * 8 + y] = y + 1;
    }
        
/*    for( int x = 0; x < 6; x++ )
    {
        TC0100SCN->bg1_colscroll[20 + x] = sine_wave[(frame_count*2+(x*8)) & 0xff] >> 4;
    }*/
    for( int y = 0; y < 16; y++ )
    {
//        TC0100SCN->bg1_rowscroll[20 * 8 + y] = sine_wave[(frame_count*2+(y*4)) & 0xff] >> 4;
    }

    on_layer(FG0);
    pen_color(0);
    move_to(3, 2);
    print("VBL: %05X  FRAME: %05X\n", vblank_count, frame_count);

    frame_count++;
}

int16_t invalid_read_count;

void init_scn_align()
{
    reset_screen();

    // Max extent corner boundaries
    on_layer(BG0); pen_color(0);
    sym_at(1, 1, 1);
    sym_at(1, 28, 1);
    sym_at(40, 1, 1);
    sym_at(40, 28, 1);

    // Should not be visible
    pen_color(1);
    sym_at(0, 1, 0x1b);
    sym_at(1, 0, 0x1b);
    sym_at(0, 28, 0x1b);
    sym_at(1, 29, 0x1b);
    sym_at(40, 0, 0x1b);
    sym_at(41, 1, 0x1b);
    sym_at(40, 29, 0x1b);
    sym_at(41, 28, 0x1b);

    pen_color(6);
    print_at(4, 3, "LAYER BG0");
    on_layer(BG1);
    pen_color(3);
    print_at(4, 4, "LAYER BG1");
    on_layer(FG0);
    pen_color(0);
    print_at(4, 5, "LAYER FG0");

    on_layer(BG0);
    pen_color(9);
    print_at(10, 8, "NO SCROLL");
    print_at(10, 9, "NO SCROLL");
    print_at(10, 20, "NO SCROLL");
    print_at(10, 21, "NO SCROLL");
    for (int y = 0; y < 10; y++)
    {
        sym_at(8, 10 + y, 0x12);
        print_at(10, 10 + y, "%d SCROLL", y);
    }

#if HAS_TC0100SCN
    for( int y = 0; y < 10 * 8; y++ )
    {
        TC0100SCN->bg0_rowscroll[(8 * 10) + y] = y - 39;
    }
#endif

    frame_count = 0;
    invalid_read_count = 0;
}

void update_scn_align()
{
    wait_vblank();

    bool flip = 0; //(input_dsw() & 0x02) == 0;

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

#if HAS_TC0100SCN
    TC0100SCN_Ctrl->bg1_y = base_y;
    TC0100SCN_Ctrl->bg1_x = base_x;
    TC0100SCN_Ctrl->fg0_y = base_y;
    TC0100SCN_Ctrl->fg0_x = base_x;
    TC0100SCN_Ctrl->system_flags = system_flags;
    TC0100SCN_Ctrl->layer_flags = 0;
    TC0100SCN_Ctrl->bg0_y = base_y;
    TC0100SCN_Ctrl->bg0_x = base_x;
#endif

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;
    uint16_t cmd_base = OBJCMD_6BPP;

    if (flip) cmd_base |= OBJCMD_FLIPSCREEN;

    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 100, 30); obj_commit_reset(o, &obj_ptr);
  
    GridOptions opt;
    opt.w = 2; opt.h = 2;
    opt.extra = opt.zoom = 0b100'000'000;
    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;
    opt.zoom_x = 0; opt.zoom_y = 0;
    opt.pos = 0;
    obj_grid(135, 40, &opt, &obj_ptr);

    opt.w = 1; opt.h = 1;
    opt.extra = opt.zoom = 1;
    opt.latch_x = opt.latch_y = 0;
    opt.seq = 0;
    opt.zoom_x = 0; opt.zoom_y = 0;
    opt.pos = 0;
    obj_grid(8, -15, &opt, &obj_ptr);
    obj_grid(8, 200, &opt, &obj_ptr);


    for( int i = 0; i < 512; i++ )
    {
        obj_ptr[i].pos0 = frame_count;
    }
    for( int i = 0; i < 512; i++ )
    {
        if( obj_ptr[i].pos0 != frame_count )
        {
            *(uint32_t *)0xff0000 = 1;
            invalid_read_count++;
        }
    }

    frame_count++;

    on_layer(BG0);
    move_to(20, 1);
    print("%u %u", frame_count, invalid_read_count);
}



uint16_t system_flags, layer_flags;

void init_scn_control_access()
{
    reset_screen();

    frame_count = 0;
    system_flags = 0;
    layer_flags = 0;

    const char *msg1 = "UP/DOWN ADJUST SYSTEM FLAGS";
    const char *msg2 = "LEFT/RIGHT ADJUST LAYER FLAGS";

    pen_color(0);
    on_layer(BG0);
    print_at(4, 20, msg1);
    on_layer(BG1);
    print_at(4, 20, msg1);
    on_layer(FG0);
    print_at(4, 20, msg1);

    on_layer(BG0);
    print_at(4, 21, msg2);
    on_layer(BG1);
    print_at(4, 21, msg2);
    on_layer(FG0);
    print_at(4, 21, msg2);
}

void update_scn_control_access()
{
    bool changed = false;

    if(input_pressed(LEFT))
    {
        if (system_flags == 0)
            system_flags = 1;
        else
            system_flags = system_flags << 1;
        changed = true;
    }

    if(input_pressed(RIGHT))
    {
        if (system_flags == 0)
            system_flags = 1 << 15;
        else
            system_flags = system_flags >> 1;
        changed = true;
    }

    if(input_pressed(UP))
    {
        if (layer_flags == 0)
            layer_flags = 1;
        else
            layer_flags = layer_flags << 1;
        changed = true;
    }

    if(input_pressed(DOWN))
    {
        if (layer_flags == 0)
            layer_flags = 1 << 15;
        else
            layer_flags = layer_flags >> 1;
        changed = true;
    }

#if HAS_TC0100SCN
    TC0100SCN_Ctrl->system_flags = system_flags;
    TC0100SCN_Ctrl->layer_flags = layer_flags;
#endif

    if (changed || frame_count == 0)
    {
        pen_color(0);
        on_layer(BG0);
        print_at(4, 4, "LAYER: %06X", layer_flags);
        on_layer(BG1);
        print_at(4, 5, "LAYER: %06X", layer_flags);
        on_layer(FG0);
        print_at(4, 6, "LAYER: %06X", layer_flags);

        pen_color(0);
        on_layer(BG0);
        print_at(20, 4, "SYSTEM: %06X", system_flags);
        on_layer(BG1);
        print_at(20, 5, "SYSTEM: %06X", system_flags);
        on_layer(FG0);
        print_at(20, 6, "SYSTEM: %06X", system_flags);
    }

    frame_count++;
}

void init_obj_general()
{
    reset_screen();

    frame_count = 0;
}

uint16_t obj_test_data[] =
{
    0x0000, 0x0000, 0x0000, 0x8000, 0x0000, 0x0300, 0x0000, 0x0000,
    0x0000, 0x0000, 0xA000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x50D0, 0x1058, 0x090C, 0x0000, 0x0000, 0x0000,
    0x13D3, 0x0000, 0x0000, 0x0000, 0x3D00, 0x0000, 0x0000, 0x0000,
    0x13D7, 0x0000, 0x0000, 0x0000, 0x3D00, 0x0000, 0x0000, 0x0000,
    0x13CF, 0x0000, 0x0000, 0x0000, 0xCD00, 0x0000, 0x0000, 0x0000,
    0x13D2, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13D6, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13CE, 0x0000, 0x0000, 0x0000, 0xCD00, 0x0000, 0x0000, 0x0000,
    0x13D1, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13D5, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0xCD00, 0x0000, 0x0000, 0x0000,
    0x13D0, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13D4, 0x0000, 0x0000, 0x0000, 0x7500, 0x0000, 0x0000, 0x0000,
    0x0000, 0x1f1f, 0x5130, 0x0058, 0x090C, 0x0000, 0x0000, 0x0000,
    0x13D3, 0x0000, 0x0000, 0x0000, 0x3D00, 0x0000, 0x0000, 0x0000,
    0x13D7, 0x0000, 0x0000, 0x0000, 0x3D00, 0x0000, 0x0000, 0x0000,
    0x13CF, 0x0000, 0x0000, 0x0000, 0xCD00, 0x0000, 0x0000, 0x0000,
    0x13D2, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13D6, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13CE, 0x0000, 0x0000, 0x0000, 0xCD00, 0x0000, 0x0000, 0x0000,
    0x13D1, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13D5, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0xCD00, 0x0000, 0x0000, 0x0000,
    0x13D0, 0x0000, 0x0000, 0x0000, 0x7D00, 0x0000, 0x0000, 0x0000,
    0x13D4, 0x0000, 0x0000, 0x0000, 0x7500, 0x0000, 0x0000, 0x0000,
};

void update_obj_general()
{
    wait_dma();
    memcpyw(TC0200OBJ, obj_test_data, sizeof(obj_test_data) / 2);
    wait_dma();

    // Max extent corner boundaries
    on_layer(BG0); pen_color(0);
    sym_at(1, 1, 1);
    sym_at(1, 28, 1);
    sym_at(40, 1, 1);
    sym_at(40, 28, 1);

    on_layer(BG0); pen_color(2);
    sym_at(17, 11, 1);
    sym_at(22, 13, 1);
    sym_at(17, 16, 1);
    sym_at(20, 16, 1);

    pen_color(1);
    sym_at(18, 15, 0x1b);


    // Should not be visible
    pen_color(1);
    sym_at(0, 1, 0x1b);
    sym_at(1, 0, 0x1b);
    sym_at(0, 28, 0x1b);
    sym_at(1, 29, 0x1b);
    sym_at(40, 0, 0x1b);
    sym_at(41, 1, 0x1b);
    sym_at(40, 29, 0x1b);
    sym_at(41, 28, 0x1b);


    frame_count++;
}


void send_sound_code(uint8_t code)
{
    *SYT_ADDR = 0;
    *SYT_DATA = (code >> 4) & 0xf;
    *SYT_ADDR = 1;
    *SYT_DATA = code & 0xf;
    *SYT_ADDR = 2;
    *SYT_DATA = (code >> 4) & 0xf;
    *SYT_ADDR = 3;
    *SYT_DATA = code & 0xf;
}

bool read_sound_response(uint8_t *code)
{
    uint8_t r = 0;
    *SYT_ADDR = 4;

    uint8_t status = *SYT_DATA;

    if (status & 0x4)
    {
        *SYT_ADDR = 0;
        r = (*SYT_DATA << 4);
        *SYT_ADDR = 1;
        r |= *SYT_DATA;

        if (code) *code = r;
        return true;
    }

    return false;
}

static uint8_t sound_code = 0;
static uint8_t sound_msg = 0;
static uint8_t sound_msg2 = 0;

static uint8_t sound_data[16];

void init_sound_test()
{
    reset_screen();
    
    // Reset
    *SYT_ADDR = 4;
    *SYT_DATA = 1;


    *SYT_ADDR = 4;
    *SYT_DATA = 0;


/*    wait_vblank();
    wait_vblank();
    wait_vblank();
    send_sound_code(0xFE);
    wait_vblank();
    wait_vblank();
    wait_vblank();
    wait_vblank();
    send_sound_code(0xA0);
    sound_code = 0xA0;*/
}

void update_sound_test()
{
    wait_vblank();

    if(input_pressed(LEFT))
    {
        sound_code--;
    }

    if(input_pressed(RIGHT))
    {
        sound_code++;
    }

    if(input_pressed(BTN1))
    {
        //send_sound_code(sound_code);
        extern void run_mdfourier();
        run_mdfourier();
    }
        
    *SYT_ADDR = 4;
    uint8_t status = (*SYT_DATA) & 0xf;

    if(status & 0x04)
    {
        *SYT_ADDR = 0;
        sound_msg = (*SYT_DATA) << 4;
        *SYT_ADDR = 1;
        sound_msg |= (*SYT_DATA) & 0x0f;
    }

    if(status & 0x08)
    {
        *SYT_ADDR = 2;
        sound_msg2 = (*SYT_DATA << 4);
        *SYT_ADDR = 3;
        sound_msg2 |= (*SYT_DATA & 0x0f);
    }

    if(input_pressed(BTN3))
    {
        pen_color(1);
        on_layer(FG0);
        move_to(4, 10);
        for( int i = 0; i < 16; i++ )
        {
            *SYT_ADDR = i;
            sound_data[i] = *SYT_DATA;
            print("%01X ", sound_data[i] & 0xf);
        }
    }

    pen_color(0);
    on_layer(FG0);
    print_at(4, 4, "SOUND CODE: %02X", sound_code);
    print_at(4, 5, "STATUS: %02X", status);
    print_at(4, 6, "MESSAGE: %02X %02X", sound_msg, sound_msg2);
}

void init_obj_test1()
{
    reset_screen();
}

void update_obj_test1()
{
    uint16_t cmd_base = OBJCMD_6BPP;
    wait_dma();

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;

    obj_reset(o);
    if (input_down(BTN1))
    {
        TC0200OBJ[4095].code = 0;
        obj_cmd(o, cmd_base);  obj_commit_reset(o, &obj_ptr);
    }
    else
    {   
        obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    }
    obj_master_xy(o, 100, 30); obj_commit_reset(o, &obj_ptr);
  
    GridOptions opt;
    opt.w = 3; opt.h = 3;
    opt.extra = opt.zoom = 0b100'000'000;
    opt.zoom_x = 0; opt.zoom_y = 0;
    opt.pos = 0;

    // "Standard" way
    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(10, 10, &opt, &obj_ptr);

    // No sequence flags
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(70, 10, &opt, &obj_ptr);

    // No inc y flags
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(130, 10, &opt, &obj_ptr);
    
    // Latch x always flags
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b011'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(190, 10, &opt, &obj_ptr);

    obj_master_xy(o, 100, 90); obj_commit_reset(o, &obj_ptr);

    // Now with zoom
    opt.zoom_x = 126; opt.zoom_y = 126;
    // "Standard" way
    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(10, 10, &opt, &obj_ptr);

    // No sequence flags
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(70, 10, &opt, &obj_ptr);

    // No inc y flags
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(130, 10, &opt, &obj_ptr);

    // Latch x always flags
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b011'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(190, 10, &opt, &obj_ptr);
        
    obj_master_xy(o, 100, 150); obj_commit_reset(o, &obj_ptr);
    
    // Now with zoom and always set seq
    // No inc y flags
    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(130, 10, &opt, &obj_ptr);

    // Latch x always flags
    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b011'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(190, 10, &opt, &obj_ptr);
    
    
    obj_ptr = TC0200OBJ + 2049;
    obj_reset(o);
    obj_cmd(o, cmd_base); o->pos1 = 0x8000; obj_commit_reset(o, &obj_ptr);

}

void init_obj_test2()
{
    reset_screen();
}

void update_obj_test2()
{
    uint16_t cmd_base = OBJCMD_6BPP;
    wait_dma();

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;

    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 100, 30); obj_commit_reset(o, &obj_ptr);

    GridOptions opt;
    opt.w = 3; opt.h = 3;
    opt.zoom_x = 0; opt.zoom_y = 0;
    opt.zoom = 0b100'000'000;
    opt.extra = 0b100'000'000;
    opt.pos = 0;

    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'111'111; opt.inc_y = 0b011'111'111;
    opt.latch_x = 0b011'000'100; opt.inc_x = 0b011'111'001;   
    obj_grid(10, 10, &opt, &obj_ptr);

    opt.seq = 0b000'000'000;
    opt.latch_y = 0b111'011'011; opt.inc_y = 0b111'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(70, 10, &opt, &obj_ptr);

    obj_master_xy(o, 300, 30); obj_commit_reset(o, &obj_ptr);
    obj_extra_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);

    opt.extra = 0b010'000'000;
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b001'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(20, 20, &opt, &obj_ptr);
/*
    opt.seq = 0b000'000'000;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b011'111'111; opt.inc_x = 0b000'100'100;   
    obj_grid(190, 10, &opt, &obj_ptr);
*/
}

int8_t test3_index;
void init_obj_test3()
{
    test3_index = 0;
    reset_screen();
}

#define IF_TEST(idx) if (idx == test3_index || test3_index == 0)
void update_obj_test3()
{
    uint16_t cmd_base = OBJCMD_6BPP;
    wait_dma();

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;

    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 140, 70); obj_commit_reset(o, &obj_ptr);
    obj_extra_xy(o, -40, -40); obj_commit_reset(o, &obj_ptr);
 
    if (input_pressed(LEFT))
    {
        test3_index--;
    }

    if (input_pressed(RIGHT))
    {
        test3_index++;
    }

    if (test3_index > 10) test3_index = 0;
    if (test3_index < 0) test3_index = 10;

    GridOptions opt;
    opt.w = 3; opt.h = 3;
    opt.extra = 0;
    opt.pos = opt.zoom = 0b100'000'000;
    opt.zoom_x = 0; opt.zoom_y = 0;

    IF_TEST(1)
    {
        // "Standard" way
        opt.seq = 0b111'111'110;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(10, 10, &opt, &obj_ptr);
    }

    IF_TEST(2)
    {
        // No sequence flags
        opt.seq = 0b000'000'000;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(70, 10, &opt, &obj_ptr);
    }

    IF_TEST(3)
    {
        // No inc y flags
        opt.seq = 0b000'000'000;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(130, 10, &opt, &obj_ptr);
    }
        
    IF_TEST(4)
    {
        // Latch x always flags
        opt.seq = 0b000'000'000;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
        opt.latch_x = 0b011'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(190, 10, &opt, &obj_ptr);
    }

    obj_master_xy(o, 140, 130); obj_commit_reset(o, &obj_ptr);

    // Now with zoom
    opt.zoom_x = 126; opt.zoom_y = 126;

    IF_TEST(5)
    {
        // "Standard" way
        opt.seq = 0b111'111'110;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(10, 10, &opt, &obj_ptr);
    }

    IF_TEST(6)
    {
        // No sequence flags
        opt.seq = 0b000'000'000;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(70, 10, &opt, &obj_ptr);
    }

    IF_TEST(7)
    {
        // No inc y flags
        opt.seq = 0b000'000'000;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(130, 10, &opt, &obj_ptr);
    }

    IF_TEST(8)
    {
        // Latch x always flags
        opt.seq = 0b000'000'000;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
        opt.latch_x = 0b011'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(190, 10, &opt, &obj_ptr);
    }
            
    obj_master_xy(o, 140, 190); obj_commit_reset(o, &obj_ptr);
    
    IF_TEST(9)
    {
        // Now with zoom and always set seq
        // No inc y flags
        opt.seq = 0b111'111'110;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(130, 10, &opt, &obj_ptr);
    }

    IF_TEST(10)
    {
        // Latch x always flags
        opt.seq = 0b111'111'110;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b000'000'000;
        opt.latch_x = 0b011'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(190, 10, &opt, &obj_ptr);
    }
    
    obj_cmd(o, cmd_base | OBJCMD_DISABLE); obj_commit_reset(o, &obj_ptr);
}

void init_basic_timing()
{
    reset_screen();
}

void update_basic_timing()
{
    uint32_t increment = 0;
    uint32_t scn_write = 0;
    uint32_t obj_write = 0;
    uint32_t dar_write = 0;
    uint32_t roz_write = 0;

#if HAS_TC0480SCP
    volatile uint16_t *scn_addr = &TC0480SCP->bg0[0].code;
#else
    volatile uint16_t *scn_addr = &TC0100SCN->bg0[0].code;
#endif    
    volatile uint16_t *obj_addr = &TC0200OBJ[64].pos0;

#if HAS_TC0260DAR
    volatile uint8_t *dar_addr = (uint8_t *)&TC0260DAR[128];
#endif //HAS_TC0260DAR

#if HAS_TC0430GRW
    volatile uint16_t *roz_addr = &TC0430GRW[128];
#endif

    uint32_t vb = vblank_count;
    while( vb == vblank_count ) {}
    vb = vblank_count;
    
    while( vb == vblank_count )
    {
        increment++;
    }
    vb = vblank_count;

    while( vb == vblank_count )
    {
        *scn_addr = 0x0000;
        scn_write++;
    }
    vb = vblank_count;

    while( vb == vblank_count )
    {
        *obj_addr = 0x0000;
        obj_write++;
    }
    vb = vblank_count;

#if HAS_TC0260DAR
    while( vb == vblank_count )
    {
        *dar_addr = 0x0000;
        dar_write++;
    }
    vb = vblank_count;
#endif

#if HAS_TC0430GRW
    while( vb == vblank_count )
    {
        *roz_addr = 0x0000;
        roz_write++;
    }
    vb = vblank_count;
#endif 

    pen_color(0);
    on_layer(FG0);
    move_to(4, 4);
    print("INCREMENT: %08X\n", increment);
    print("SCN_WRITE: %08X\n", scn_write);
    print("OBJ_WRITE: %08X\n", obj_write);
    print("DAR_WRITE: %08X\n", dar_write);
    print("ROZ_WRITE: %08X\n", roz_write);
}

void init_360pri()
{
    reset_screen();

    fg0_gfx(2);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(1, 0, 0, 0, 0, 0, 0, 1);
    fg0_row(1, 0, 2, 2, 2, 2, 0, 1);
    fg0_row(1, 0, 2, 3, 3, 2, 0, 1);
    fg0_row(1, 0, 2, 3, 3, 2, 0, 1);
    fg0_row(1, 0, 2, 2, 2, 2, 0, 1);
    fg0_row(1, 0, 0, 0, 0, 0, 0, 1);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);

    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(2, 2, 2, 2, 2, 2, 2, 2);
    fg0_row(2, 2, 2, 2, 2, 2, 2, 2);
    fg0_row(3, 3, 3, 3, 3, 3, 3, 3);
    fg0_row(3, 3, 3, 3, 3, 3, 3, 3);
    fg0_row(3, 3, 3, 3, 3, 3, 3, 3);

    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);

    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);

    tc0360pri_set_roz_prio(4, 4, 4, 4);
    tc0360pri_set_roz(0, 4 << 2);
    
    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;
    uint16_t cmd_base = 0;
    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 128, 23); obj_commit_reset(o, &obj_ptr);
 
    GridOptions opt;
    memset(&opt, 0, sizeof(opt));
    opt.w = 1; opt.h = 1;
    opt.extra = opt.zoom = 0b1;
    opt.tile = 0x7AE;
    
    for( int y = 0; y < 4; y++ )
    {
        for (int x = 0; x < 4; x++ )
        {
            opt.color = 24 + y + (64 * x);
            obj_grid(x * 24, y * 24, &opt, &obj_ptr);
            on_layer(FG0);
            pen_color(28);
            sym_at(5 + (3 * x), 2 + (3 * y), 3);
            sym_at(18 + (3 * x), 2 + (3 * y), 3);
            pen_color(29);
            sym_at(6 + (3 * x), 2 + (3 * y), 3);
            sym_at(19 + (3 * x), 2 + (3 * y), 3);
            pen_color(30);
            sym_at(5 + (3 * x), 3 + (3 * y), 3);
            sym_at(18 + (3 * x), 3 + (3 * y), 3);
            pen_color(31);
            sym_at(6 + (3 * x), 3 + (3 * y), 3);
            sym_at(19 + (3 * x), 3 + (3 * y), 3);
             
            on_layer(ROZ);
            pen_color(3);
            sym_at(19 + (3 * x), 2 + (3 * y), 0x67);
            sym_at(18 + (3 * x), 3 + (3 * y), 0x67);
        }
    }

    opt.color = 16;
    obj_grid(10 * 24, 0 * 24, &opt, &obj_ptr);

    on_layer(FG0);

    for( int y = 0; y < 8; y++ )
    {
        pen_color(24 + y);
        sym_at(31, y+2, 0x1);
    }
}

#define WRAP(v, mx) v = (v >= 255 || v < 0) ? (mx) : (v > (mx)) ? (0) : v

void update_360pri()
{
    static int sel = 0;

    static uint8_t fg_prio = 6;
    static uint8_t obj0_prio = 2;
    static uint8_t obj1_prio = 3;
    static uint8_t obj2_prio = 5;
    static uint8_t obj3_prio = 7;
    static uint8_t roz_prio = 4;
    static uint8_t black_out = 0;
    static uint8_t white_out = 0;

    static uint8_t blend_mode = 0;
    static uint8_t dar_flag = 0;

    wait_vblank();

    on_layer(FG0);
    pen_color(8);
    move_to(4, 15);

    print("%c FG: %X\n", sel == 0 ? '*' : ' ', fg_prio);
    print("%c O1: %X\n", sel == 1 ? '*' : ' ', obj0_prio);
    print("%c O2: %X\n", sel == 2 ? '*' : ' ', obj1_prio);
    print("%c O3: %X\n", sel == 3 ? '*' : ' ', obj2_prio);
    print("%c O4: %X\n", sel == 4 ? '*' : ' ', obj3_prio);
    print("%c RZ: %X\n", sel == 5 ? '*' : ' ', roz_prio);

    move_to(15, 15);
    for( int x = 0; x < 8; x++ )
    {
        print("%c BM%d: %X\n", sel == (6 + x) ? '*' : ' ', x, (blend_mode >> x) & 0x1);
    }
    
    move_to(24,15);
    print("%c BLACK: %02X\n", sel == 14 ? '*' : ' ', black_out);
    print("%c WHITE: %02X\n", sel == 15 ? '*' : ' ', white_out);
    print("%c DAR: %X\n", sel == 16 ? '*' : ' ', dar_flag);


    int adj = 0;
    bool color_change = false;
    bool dar_change = false;
    if (input_pressed(LEFT)) adj = -1;
    if (input_pressed(RIGHT)) adj = 1;

    switch(sel)
    {
        case 0: fg_prio += adj; break;
        case 1: obj0_prio += adj; break;
        case 2: obj1_prio += adj; break;
        case 3: obj2_prio += adj; break;
        case 4: obj3_prio += adj; break;
        case 5: roz_prio += adj; break;
        case 14: black_out += adj; color_change = adj != 0; break;
        case 15: white_out += adj; color_change = adj != 0; break;
        case 16: dar_flag += adj; dar_change = adj != 0; break;
        default: break;
    }

    if (adj != 0 && sel >= 6 && sel <= 13)
    {
        blend_mode ^= (1 << (sel - 6));
    }

    if (input_pressed(DOWN)) sel++;
    if (input_pressed(UP)) sel--;

    WRAP(sel, 16);
    WRAP(fg_prio, 15);
    WRAP(obj0_prio, 15);
    WRAP(obj1_prio, 15);
    WRAP(obj2_prio, 15);
    WRAP(obj3_prio, 15);
    WRAP(roz_prio, 15);
    WRAP(dar_flag, 1);

    if (color_change)
    {
        set_default_palette();
        set_gradient2(white_out, 255, 255, 255, 255, 255, 255);
        set_gradient2(black_out, 0, 0, 0, 0, 0, 0);
    }

    if (dar_change)
    {
        *(uint16_t *)0x500000 = dar_flag ? 0x0100 : 0x0000;
    }

    tc0360pri_set_roz_prio(roz_prio, roz_prio, roz_prio, roz_prio);
    tc0360pri_set_obj_prio(obj0_prio, obj1_prio, obj2_prio, obj3_prio);
    tc0360pri_set_tile_prio(fg_prio, 0, 0);
    tc0360pri_set_blendmode(blend_mode);
}

void init_480scp()
{
    reset_screen();

    // 0 - BG3
    // 1 - FG0 / BG0
    // 2 - BG1
    // 3 - BG2
    tc0360pri_set_tile_prio2(4, 4, 4, 4);

    // 0 - BG1
    // 1 - BG0 / BG2
    // 2 - BG3
    // 3 - FG0
    tc0360pri_set_roz_prio(5, 5, 5, 5);
    tc0360pri_set_roz(0, 0);
    
    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;
    uint16_t cmd_base = 0;
    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 128, 23); obj_commit_reset(o, &obj_ptr);
 
    GridOptions opt;
    memset(&opt, 0, sizeof(opt));
    opt.w = 1; opt.h = 1;
    opt.extra = opt.zoom = 0b1;
    opt.tile = (0x1186 & 0x7ff) | ( 1 << 11 );
    
    opt.color = 24;
    obj_grid(2 * 24, 2 * 24, &opt, &obj_ptr);

    // BG0 = 0010
    // BG1 = 0100
    // BG2 = 0110
    // BG3 = 1000
    // FG0 = 1010

    on_layer(FG0);

    for( int y = 0; y < 8; y++ )
    {
        pen_color(24 + y);
        sym_at(31, y+2, 0x1);
    }

    pen_color(9);
    sym_at(1,0,1);
    sym_at(0,1,1);
    sym_at(52,28,1);
    sym_at(51,29,1);

    pen_color(8);
    sym_at(1,1,1);
    sym_at(51,28,1);


    on_layer(BG0);
    pen_color(9);
    sym_at(1,15,1);
    sym_at(0,14,1);
    sym_at(20,0,1);
    sym_at(21,1,1);

    pen_color(8);
    sym_at(1, 14, 1);
    sym_at(20, 1, 1);

    pen_color(25);
    on_layer(BG0);
    sym_at(3, 4, 0x10);
    sym_at(13, 4, 0x10);

    pen_color(26);
    on_layer(BG1);
    sym_at(3, 5, 0x10);
    sym_at(13, 5, 0x10);

    pen_color(27 | 0x8000);
    on_layer(BG2);
    sym_at(3, 6, 0x10);
    sym_at(13, 6, 0x10);

    pen_color(28);
    on_layer(BG3);
    sym_at(3, 7, 0x10);
    sym_at(13, 7, 0x10);

    on_layer(FG0);
    pen_color(0x08);
    sym_at(5, 5, 'F');
    pen_color(0x88);
    sym_at(5, 6, 'F');
    pen_color(0x48);
    sym_at(6, 5, 'F');
    pen_color(0xC8);
    sym_at(6, 6, 'F');

    /*
    on_layer(BG3);
    uint16_t flags = 0;
    for( int y = 0; y < 8; y++ )
    {
        for (int x = 0; x < 8; x++ )
        {
            pen_color(255 | (flags << 8));
            sym_at(x + 4, y + 4, 0x8025);
            flags++;
        }
    }*/

}

void update_480scp()
{
    wait_vblank();
    pen_color(8);
    on_layer(FG0);
    sym_at(10,10,1);
    
    print_at(2,3,"VBLANK: %04X", vblank_count);
}

void init_480scp_zoom()
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
void update_480scp_zoom()
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

void init_480scp_row()
{
    reset_screen();

    // 0 - BG3
    // 1 - FG0 / BG0
    // 2 - BG1
    // 3 - BG2
    tc0360pri_set_tile_prio2(4, 4, 4, 4);

    // 0 - BG1
    // 1 - BG0 / BG2
    // 2 - BG3
    // 3 - FG0
    tc0360pri_set_roz_prio(5, 5, 5, 5);
    tc0360pri_set_roz(0, 0);
    
    on_layer(FG0);
    pen_color(1);
    print_at(20, 4, "BG0");
    print_at(20, 10, "BG1");
    print_at(20, 16, "BG2");
    print_at(20, 22, "BG3");

    // BG0
    TC0480SCP_Ctrl->bg0_x = 40;
    TC0480SCP_Ctrl->bg0_y = -100;
    TC0480SCP_Ctrl->bg0_zoom = 0x8000;

    TC0480SCP->bg0_rowscroll[32] = -10;
    TC0480SCP->bg0_rowscroll[33] = 32;
    TC0480SCP->bg0_rowscroll[34] = -10;
    TC0480SCP->bg0_rowscroll[35] = 32;

    on_layer(BG0);
    pen_color(0x1b);
    sym_at(1, 2, 0x19);
    sym_at(5, 2, 1);

    // BG2
    TC0480SCP_Ctrl->system_flags = TC0480SCP_SYSTEM_EXT_SYNC | TC0480SCP_SYSTEM_BG2_ZOOM;
    TC0480SCP_Ctrl->bg2_x = 40;
    TC0480SCP_Ctrl->bg2_y = -300;
    TC0480SCP_Ctrl->bg2_zoom = 0x8000;
    
    TC0480SCP->bg2_rowscroll[32] = -10;
    TC0480SCP->bg2_rowscroll[33] = 32;
    TC0480SCP->bg2_rowscroll[34] = -10;
    TC0480SCP->bg2_rowscroll[35] = 32;
    TC0480SCP->bg2_zoom[32] = 128;
    TC0480SCP->bg2_zoom[33] = 32;
    TC0480SCP->bg2_zoom[34] = 128;
    TC0480SCP->bg2_zoom[35] = 32;
    TC0480SCP->bg2_zoom[36] = 80;
   
    for ( int x = 20; x < 30; x++ )
        TC0480SCP->bg2_rowscroll[x] = x;
    TC0480SCP->bg2_colscroll[0xa4] = 10;


    on_layer(BG2);
    pen_color(0x1c);
    sym_at(1, 2, 0x19);
    sym_at(5, 2, 1);
    pen_color(0x8);
    sym_at(3, 1, 2);
}

void update_480scp_row()
{
    static int x = 0;
    wait_vblank();
    pen_color(8);
    on_layer(FG0);
    
    print_at(2,3,"VBLANK: %04X %04X", vblank_count, TC0480SCP->fg0[400].attr_code);
   
    TC0480SCP->fg0[400].attr_code += 0x0013;

    //TC0480SCP->bg2_colscroll[x] = 0;
    //x = (x + 1) % 512;
    //TC0480SCP->bg2_colscroll[x] = -10;
}


static int fail_index = 0;

void init_romtest()
{
    reset_screen();

    on_layer(FG0);
    pen_color(8);
    print_at(1,1,"ROM TEST");
}

const uint16_t rom_data[256] __attribute__ ((aligned (1024))) =
{
    0x00ff, 0x01fe, 0x02fd, 0x03fc, 0x04fb, 0x05fa, 0x06f9, 0x07f8, 0x08f7, 0x09f6, 0x0af5, 0x0bf4, 0x0cf3, 0x0df2, 0x0ef1, 0x0ff0,
    0x10ef, 0x11ee, 0x12ed, 0x13ec, 0x14eb, 0x15ea, 0x16e9, 0x17e8, 0x18e7, 0x19e6, 0x1ae5, 0x1be4, 0x1ce3, 0x1de2, 0x1ee1, 0x1fe0,
    0x20df, 0x21de, 0x22dd, 0x23dc, 0x24db, 0x25da, 0x26d9, 0x27d8, 0x28d7, 0x29d6, 0x2ad5, 0x2bd4, 0x2cd3, 0x2dd2, 0x2ed1, 0x2fd0,
    0x30cf, 0x31ce, 0x32cd, 0x33cc, 0x34cb, 0x35ca, 0x36c9, 0x37c8, 0x38c7, 0x39c6, 0x3ac5, 0x3bc4, 0x3cc3, 0x3dc2, 0x3ec1, 0x3fc0,
    0x40bf, 0x41be, 0x42bd, 0x43bc, 0x44bb, 0x45ba, 0x46b9, 0x47b8, 0x48b7, 0x49b6, 0x4ab5, 0x4bb4, 0x4cb3, 0x4db2, 0x4eb1, 0x4fb0,
    0x50af, 0x51ae, 0x52ad, 0x53ac, 0x54ab, 0x55aa, 0x56a9, 0x57a8, 0x58a7, 0x59a6, 0x5aa5, 0x5ba4, 0x5ca3, 0x5da2, 0x5ea1, 0x5fa0,
    0x609f, 0x619e, 0x629d, 0x639c, 0x649b, 0x659a, 0x6699, 0x6798, 0x6897, 0x6996, 0x6a95, 0x6b94, 0x6c93, 0x6d92, 0x6e91, 0x6f90,
    0x708f, 0x718e, 0x728d, 0x738c, 0x748b, 0x758a, 0x7689, 0x7788, 0x7887, 0x7986, 0x7a85, 0x7b84, 0x7c83, 0x7d82, 0x7e81, 0x7f80,
    0x807f, 0x817e, 0x827d, 0x837c, 0x847b, 0x857a, 0x8679, 0x8778, 0x8877, 0x8976, 0x8a75, 0x8b74, 0x8c73, 0x8d72, 0x8e71, 0x8f70,
    0x906f, 0x916e, 0x926d, 0x936c, 0x946b, 0x956a, 0x9669, 0x9768, 0x9867, 0x9966, 0x9a65, 0x9b64, 0x9c63, 0x9d62, 0x9e61, 0x9f60,
    0xa05f, 0xa15e, 0xa25d, 0xa35c, 0xa45b, 0xa55a, 0xa659, 0xa758, 0xa857, 0xa956, 0xaa55, 0xab54, 0xac53, 0xad52, 0xae51, 0xaf50,
    0xb04f, 0xb14e, 0xb24d, 0xb34c, 0xb44b, 0xb54a, 0xb649, 0xb748, 0xb847, 0xb946, 0xba45, 0xbb44, 0xbc43, 0xbd42, 0xbe41, 0xbf40,
    0xc03f, 0xc13e, 0xc23d, 0xc33c, 0xc43b, 0xc53a, 0xc639, 0xc738, 0xc837, 0xc936, 0xca35, 0xcb34, 0xcc33, 0xcd32, 0xce31, 0xcf30,
    0xd02f, 0xd12e, 0xd22d, 0xd32c, 0xd42b, 0xd52a, 0xd629, 0xd728, 0xd827, 0xd926, 0xda25, 0xdb24, 0xdc23, 0xdd22, 0xde21, 0xdf20,
    0xe01f, 0xe11e, 0xe21d, 0xe31c, 0xe41b, 0xe51a, 0xe619, 0xe718, 0xe817, 0xe916, 0xea15, 0xeb14, 0xec13, 0xed12, 0xee11, 0xef10,
    0xf00f, 0xf10e, 0xf20d, 0xf30c, 0xf40b, 0xf50a, 0xf609, 0xf708, 0xf807, 0xf906, 0xfa05, 0xfb04, 0xfc03, 0xfd02, 0xfe01, 0xff00,
};

void update_romtest()
{
    on_layer(FG0);
    pen_color(8);
    print_at(20, 1, "%04X", vblank_count);
    
    TE7750->p4 = 0x00;

    if (fail_index > 20) return;

    for( uint16_t x = 0; x < 256; x++ )
    {
        uint16_t expected = ( x << 8 ) | (255 - x);
        uint16_t rom_value = rom_data[x];

        if (rom_value != expected)
        {
            TE7750->p4 = 0x20;
            print_at(1, 2 + fail_index, "%04X %04X", expected, rom_value);
            fail_index++;
        }
    }
}

void init_screen(int screen)
{
    switch(screen)
    {
        case 0: init_scn_general(); break;
        case 1: init_scn_control_access(); break;
        case 2: init_obj_general(); break;
        case 3: init_obj_test1(); break;
        case 4: init_obj_test2(); break;
        case 5: init_obj_test3(); break;
        case 6: init_sound_test(); break;
        case 7: init_scn_align(); break;
        case 8: init_basic_timing(); break;
        case 9: init_360pri(); break;
        case 10: init_480scp(); break;
        case 11: init_480scp_zoom(); break;
        case 12: init_480scp_row(); break;
        case 13: init_romtest(); break;
        default: break;
    }
}

void update_screen(int screen)
{
    switch(screen)
    {
        case 0: update_scn_general(); break;
        case 1: update_scn_control_access(); break;
        case 2: update_obj_general(); break;
        case 3: update_obj_test1(); break;
        case 4: update_obj_test2(); break;
        case 5: update_obj_test3(); break;
        case 6: update_sound_test(); break;
        case 7: update_scn_align(); break;
        case 8: update_basic_timing(); break;
        case 9: update_360pri(); break;
        case 10: update_480scp(); break;
        case 11: update_480scp_zoom(); break;
        case 12: update_480scp_row(); break;
        case 13: update_romtest(); break;
        default: break;
    }
}

void deinit_screen(int screen)
{
    switch(screen)
    {
        default: break;
    }
}

int main(int argc, char *argv[])
{
    enable_interrupts();

    input_init();

    uint32_t system_flags = 0;

    int current_screen = 12;

    init_screen(current_screen);
    
    while(1)
    {
        input_update();

        // 0x0200 // ACC MODE
        // 0x0100 // Brightness? 
        // 0x0002 // Z4
        // 0x0001 // Z3
        //
        //*(uint16_t*)0x500000 = 0x0100;

        if (input_pressed(START))
        {
            deinit_screen(current_screen);
            current_screen = ( current_screen + 1 ) % NUM_SCREENS;
            init_screen(current_screen);
        }

        update_screen(current_screen);
    }

    return 0;
}


