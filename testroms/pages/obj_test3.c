#include "../system.h"
#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"
#include "../color.h"

static int8_t test3_index;
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
    memset(&opt, 0, sizeof(opt));
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

PAGE_REGISTER(obj_test3, init_obj_test3, update_obj_test3, NULL);

