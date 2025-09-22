#include "../system.h"

#if GAME_FINALB

#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"
#include "../color.h"

TC0200OBJ_Inst *cmd_inst = NULL;

void init_obj_cmd_test()
{
    reset_screen();

    for( int i = 0; i < 4; i++ )
    {
        TC0200OBJ_Inst *obj_ptr = TC0200OBJ + (i * 1024);
        TC0200OBJ_Inst work;
        TC0200OBJ_Inst *o = &work;

        uint16_t cmd = OBJCMD_6BPP;
        uint16_t fetch = 0;

        switch(i)
        {
            case 1: fetch |= OBJCMD_A13; break;
            case 2: fetch |= OBJCMD_A14; break;
            case 3: fetch |= OBJCMD_A14 | OBJCMD_A13; break;
            default: break; 
        }

        cmd |= fetch;

        obj_reset(o);
        obj_cmd_fetch(o, cmd, fetch);
        obj_commit_reset(o, &obj_ptr);

        obj_master_xy(o, 160, 30); obj_commit_reset(o, &obj_ptr);
      
        GridOptions opt;
        opt.color = i;
        opt.tile = 0;
        opt.w = 3; opt.h = 3;
        opt.extra = opt.zoom = 0b100'000'000;
        opt.zoom_x = 0; opt.zoom_y = 0;
        opt.pos = 0;

        // "Standard" way
        opt.seq = 0b111'111'110;
        opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
        opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
        obj_grid(0, 0, &opt, &obj_ptr);

        if (i == 0)
        {
            cmd_inst = obj_ptr;
        }
        obj_reset(o);
        obj_cmd_fetch(o, cmd, fetch);
        obj_commit_reset(o, &obj_ptr);
    }
}

void update_obj_cmd_test()
{
    if (input_pressed(BTN1))
    {
        wait_dma();
        cmd_inst->cmd |= 0x8000;
    }
}

PAGE_REGISTER(obj_cmd_test, init_obj_cmd_test, update_obj_cmd_test, NULL);

#endif
