#include "../system.h"

#if GAME_FINALB

#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"
#include "../color.h"

int sel_x = 0;
int sel_y = 0;

int params[5 * 5];

void init_obj_latching()
{
    reset_screen();

    memset(params, 0, sizeof(params));
    sel_x = sel_y = 0;

    on_layer(FG0);
    pen_color(1);
    move_to(2, 20);
    print("          1 2 3 4 5\n");
    print("SEQ\n");
    print("X LATCH\n");
    print("X INC\n");
    print("Y LATCH\n");
    print("Y INC\n");
}

void update_obj_latching()
{
    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;

    //wait_vblank();

    obj_reset(o);

    obj_cmd(o, OBJCMD_6BPP); obj_commit_reset(o, &obj_ptr);
    
    obj_master_xy(o, 112, 32); obj_commit_reset(o, &obj_ptr);
    obj_extra_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);

    on_layer(FG0);
    pen_color(1);

    if (input_pressed(BTN1))
    {
        params[sel_x + (sel_y * 5)] ^= 1;
    }

    if (input_pressed(LEFT)) sel_x--;
    if (input_pressed(RIGHT)) sel_x++;
    if (input_pressed(UP)) sel_y--;
    if (input_pressed(DOWN)) sel_y++;

    if (sel_y < 0) sel_y = 4;
    if (sel_y > 4) sel_y = 0;
    if (sel_x < 0) sel_x = 4;
    if (sel_x > 4) sel_x = 0;


    for( int y = 0; y < 5; y++ )
    {
        int *p = &params[y];
        obj_xy(o, 80 + (32 * y), 10 + (32 * y));
        o->code = 0x1a4e;
        o->is_seq = p[0];
        o->latch_x = p[5];
        o->inc_x = p[10];
        o->latch_y = p[15];
        o->inc_y = p[20];
        obj_commit(o, &obj_ptr);
 
        p = &params[y * 5];
        for( int x = 0; x < 5; x++ )
        {
            if (x == sel_x && y == sel_y)
                pen_color(8);
            else
                pen_color(1);
            sym_at(12 + (x * 2), 21 + y, p[x] ? '1' : '0');
        }
    }
}

PAGE_REGISTER(obj_latching, init_obj_latching, update_obj_latching, NULL);

#endif
