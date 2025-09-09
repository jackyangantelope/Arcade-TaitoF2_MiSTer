#include "../system.h"
#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"
#include "../color.h"

static uint16_t frame_count = 0;

static void init()
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

    pen_color(3);
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

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;
    uint16_t cmd_base = OBJCMD_6BPP;
    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 80, 0); obj_commit_reset(o, &obj_ptr);
 
    GridOptions opt;
    memset(&opt, 0, sizeof(opt));
    opt.w = 1; opt.h = 1;
    opt.extra = opt.zoom = 0b1;
    opt.tile = (0x1186 & 0x7ff) | ( 1 << 11 );
    
    opt.color = 2;
    obj_grid(4, 20, &opt, &obj_ptr);
}

static void update()
{
    //wait_vblank();


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
    //print("VBL: %05X  FRAME: %05X\n", vblank_count, frame_count);

    frame_count++;
}

PAGE_REGISTER(general_100scn, init, update, NULL);

