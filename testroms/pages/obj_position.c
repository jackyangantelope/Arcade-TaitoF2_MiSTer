#include "../system.h"

#if GAME_FINALB

#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"
#include "../color.h"

void init_obj_position()
{
    reset_screen();
}

void update_obj_position()
{
    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;

    obj_reset(o);

    obj_cmd(o, 0); obj_commit_reset(o, &obj_ptr);
    
    obj_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 0, 0); o->ignore_extra = 1; obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 100, 100); obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);
    obj_extra_xy(o, 20, 20); obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 200, 200); o->latch_master = 1; obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 30, 30); o->latch_extra = 1; obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 2, 2); o->ignore_extra = 1; obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 4, 4); o->ignore_all = 1; obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 8, 8); obj_commit_reset(o, &obj_ptr);
    obj_xy(o, -100, -100); o->ignore_extra = 1; o->latch_master = 1; obj_commit_reset(o, &obj_ptr);
    obj_xy(o, 0, 0); obj_commit_reset(o, &obj_ptr);

    on_layer(FG0);
    pen_color(0);
    move_to(2, 2);
    print("   X     Y  MEIA    LX    LY\n");

    wait_dma();

    TC0200OBJ_Inst *obj = TC0200OBJ;
    while( obj != obj_ptr )
    {
        int16_t x = ((int16_t)(obj->x << 4)) >> 4;;
        int16_t y = ((int16_t)(obj->y << 4)) >> 4;;

        int16_t lx = ((int16_t)(obj->c << 4)) >> 4;;
        int16_t ly = ((int16_t)(obj->e << 4)) >> 4;;

        print("%4d  %4d  %d%d%d%d  %4d  %4d\n", x, y, obj->latch_master, obj->latch_extra, obj->ignore_extra, obj->ignore_all, lx, ly);
        obj++;
    }
}

PAGE_REGISTER(obj_position, init_obj_position, update_obj_position, NULL);

#endif
