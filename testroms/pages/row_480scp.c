#include "../system.h"

#if HAS_TC0480SCP

#include "../page.h"
#include "../util.h"
#include "../input.h"
#include "../tilemap.h"

void init_row_480scp()
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

void update_row_480scp()
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

PAGE_REGISTER(row_480scp, init_row_480scp, update_row_480scp, NULL);

#endif
