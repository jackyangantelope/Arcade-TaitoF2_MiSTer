#include "../system.h"
#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"
#include "../color.h"

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
    opt.tile = 0;
    opt.color = 0;
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

PAGE_REGISTER(obj_test2, init_obj_test2, update_obj_test2, NULL);

