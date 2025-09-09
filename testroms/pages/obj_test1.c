#include "../system.h"
#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"
#include "../color.h"

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

PAGE_REGISTER(obj_test1, init_obj_test1, update_obj_test1, NULL);

