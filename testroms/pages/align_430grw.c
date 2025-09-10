#include "../system.h"

#if HAS_TC0430GRW

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
    on_layer(FG0); pen_color(0);
    sym_at(1, 1, 1);
    sym_at(1, 28, 1);
    sym_at(40, 1, 1);
    sym_at(40, 28, 1);

    // Should not be visible
    pen_color(1);
    sym_at(0, 1, 1);
    sym_at(1, 0, 1);
    sym_at(0, 28, 1);
    sym_at(1, 29, 1);
    sym_at(40, 0, 1);
    sym_at(41, 1, 1);
    sym_at(40, 29, 1);
    sym_at(41, 28, 1);

    on_layer(ROZ);
    pen_color(1);
    sym_at(2, 1, 0x67);
    sym_at(1, 2, 0x67);
    sym_at(40, 27, 0x67);
    sym_at(39, 28, 0x67);


    sym_at(16, 16, 0x36a);
    sym_at(16, 17, 0x36a);
    sym_at(17, 16, 0x36a);
    sym_at(17, 17, 0x36a);

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;
    uint16_t cmd_base = 0;
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

    TC0430GRW_Ctrl->origin_x = -50 << 12;
    TC0430GRW_Ctrl->origin_y = -40 << 12;

    TC0430GRW_Ctrl->dxx = 1 << 10;
    TC0430GRW_Ctrl->dxy = 1 << 9;
    TC0430GRW_Ctrl->dyx = 1 << 9;
    TC0430GRW_Ctrl->dyy = 1 << 10;
    
    on_layer(FG0); pen_color(24);
    for (int y = 6; y < 25; y++)
    {
        for( int x = 16; x < 36; x++)
        {
            if( (x ^ y) & 1 ) sym_at(x, y, 1);
        }
    }
}

static void update()
{
    wait_vblank();

    on_layer(FG0);
    pen_color(0);
    move_to(3, 2);
    print("VBL: %05X  FRAME: %05X\n", vblank_count, frame_count);

    frame_count++;
}

PAGE_REGISTER(align_430grw, init, update, NULL);

#endif // #if HAS_TC0430GRW


