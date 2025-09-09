#include "../system.h"

#if HAS_TC0480SCP

#include "../page.h"
#include "../util.h"
#include "../input.h"
#include "../tilemap.h"
#include "../obj_test.h"

static void init_480scp()
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
    obj_grid(35, 49, &opt, &obj_ptr);

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
    sym_at(41,28,1);
    sym_at(40,29,1);

    pen_color(8);
    sym_at(1,1,1);
    sym_at(40,28,1);

    pen_color(8);
    on_layer(FG0);
    sym_at(10,10,1);
    sym_at(8,8,1);

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


    for( int layer = 0; layer < 4; layer++ )
    {
        on_layer(BG0 + layer);
        pen_color(0x1c + layer);
        for( int i = 0; i < 4; i++ )
        {
            sym_at( 8 + layer, 4 + i, 1);
            sym_at( 8 + i, 4 + layer, 1);
        }
    }
}

static void update_480scp()
{
    static uint16_t prio = 0;
    static uint16_t test_value = 0;
    static uint16_t prev_dsw = 0;

    uint16_t dsw = input_dsw();

    if (prev_dsw != dsw)
    {
        reset_screen_config();
        prev_dsw = dsw;
    }

    //wait_vblank();
  
    on_layer(FG0);
    pen_color(8);
    print_at(2,3,"VBLANK: %04X PRIO %02X", vblank_count, prio);

    if (input_pressed(UP)) prio++;
    if (input_pressed(DOWN)) prio--;

    prio &= 0x7;

    //TC0480SCP_Ctrl->system_flags = TC0480SCP_SYSTEM_EXT_SYNC | (prio << 2);

    test_value += 0x0101;
    for( int i = 0; i < 256; i++ )
    {
        TC0480SCP->unknown[i] += 0x0101;
    }

    for( int i = 0; i < 256; i++ )
    {
        if( TC0480SCP->unknown[i] != test_value )
        {
            TC0480SCP_Ctrl->bg1_x = 300;
        }
    }
}

PAGE_REGISTER(general_480scp, init_480scp, update_480scp, NULL);

#endif
