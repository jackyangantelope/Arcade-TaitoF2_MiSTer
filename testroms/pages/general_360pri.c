#include "../system.h"

#if HAS_TC0360PRI

#include "../page.h"
#include "../util.h"
#include "../input.h"
#include "../tilemap.h"
#include "../obj_test.h"
#include "../color.h"

void init_360pri()
{
    reset_screen();

    fg0_gfx(2);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(1, 0, 0, 0, 0, 0, 0, 1);
    fg0_row(1, 0, 2, 2, 2, 2, 0, 1);
    fg0_row(1, 0, 2, 3, 3, 2, 0, 1);
    fg0_row(1, 0, 2, 3, 3, 2, 0, 1);
    fg0_row(1, 0, 2, 2, 2, 2, 0, 1);
    fg0_row(1, 0, 0, 0, 0, 0, 0, 1);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);

    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(1, 1, 1, 1, 1, 1, 1, 1);
    fg0_row(2, 2, 2, 2, 2, 2, 2, 2);
    fg0_row(2, 2, 2, 2, 2, 2, 2, 2);
    fg0_row(3, 3, 3, 3, 3, 3, 3, 3);
    fg0_row(3, 3, 3, 3, 3, 3, 3, 3);
    fg0_row(3, 3, 3, 3, 3, 3, 3, 3);

    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);
    fg0_row(1, 1, 2, 2, 3, 3, 1, 1);

    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(1, 1, 1, 1, 3, 3, 3, 3);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);
    fg0_row(2, 2, 2, 2, 0, 0, 0, 0);

    tc0360pri_set_roz_prio(4, 4, 4, 4);
    tc0360pri_set_roz(0, 4 << 2);
    
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
    opt.tile = 0x7AE;
    
    for( int y = 0; y < 4; y++ )
    {
        for (int x = 0; x < 4; x++ )
        {
            opt.color = 24 + y + (64 * x);
            obj_grid(x * 24, y * 24, &opt, &obj_ptr);
            on_layer(FG0);
            pen_color(28);
            sym_at(5 + (3 * x), 2 + (3 * y), 3);
            sym_at(18 + (3 * x), 2 + (3 * y), 3);
            pen_color(29);
            sym_at(6 + (3 * x), 2 + (3 * y), 3);
            sym_at(19 + (3 * x), 2 + (3 * y), 3);
            pen_color(30);
            sym_at(5 + (3 * x), 3 + (3 * y), 3);
            sym_at(18 + (3 * x), 3 + (3 * y), 3);
            pen_color(31);
            sym_at(6 + (3 * x), 3 + (3 * y), 3);
            sym_at(19 + (3 * x), 3 + (3 * y), 3);
             
            on_layer(ROZ);
            pen_color(3);
            sym_at(19 + (3 * x), 2 + (3 * y), 0x67);
            sym_at(18 + (3 * x), 3 + (3 * y), 0x67);
        }
    }

    opt.color = 16;
    obj_grid(10 * 24, 0 * 24, &opt, &obj_ptr);

    on_layer(FG0);

    for( int y = 0; y < 8; y++ )
    {
        pen_color(24 + y);
        sym_at(31, y+2, 0x1);
    }
}

#define WRAP(v, mx) v = (v >= 255 || v < 0) ? (mx) : (v > (mx)) ? (0) : v

void update_360pri()
{
    static int sel = 0;

    static uint8_t fg_prio = 6;
    static uint8_t obj0_prio = 2;
    static uint8_t obj1_prio = 3;
    static uint8_t obj2_prio = 5;
    static uint8_t obj3_prio = 7;
    static uint8_t roz_prio = 4;
    static uint8_t black_out = 0;
    static uint8_t white_out = 0;

    static uint8_t blend_mode = 0;
    static uint8_t dar_flag = 0;

    wait_vblank();

    on_layer(FG0);
    pen_color(8);
    move_to(4, 15);

    print("%c FG: %X\n", sel == 0 ? '*' : ' ', fg_prio);
    print("%c O1: %X\n", sel == 1 ? '*' : ' ', obj0_prio);
    print("%c O2: %X\n", sel == 2 ? '*' : ' ', obj1_prio);
    print("%c O3: %X\n", sel == 3 ? '*' : ' ', obj2_prio);
    print("%c O4: %X\n", sel == 4 ? '*' : ' ', obj3_prio);
    print("%c RZ: %X\n", sel == 5 ? '*' : ' ', roz_prio);

    move_to(15, 15);
    for( int x = 0; x < 8; x++ )
    {
        print("%c BM%d: %X\n", sel == (6 + x) ? '*' : ' ', x, (blend_mode >> x) & 0x1);
    }
    
    move_to(24,15);
    print("%c BLACK: %02X\n", sel == 14 ? '*' : ' ', black_out);
    print("%c WHITE: %02X\n", sel == 15 ? '*' : ' ', white_out);
    print("%c DAR: %X\n", sel == 16 ? '*' : ' ', dar_flag);


    int adj = 0;
    bool color_change = false;
    bool dar_change = false;
    if (input_pressed(LEFT)) adj = -1;
    if (input_pressed(RIGHT)) adj = 1;

    switch(sel)
    {
        case 0: fg_prio += adj; break;
        case 1: obj0_prio += adj; break;
        case 2: obj1_prio += adj; break;
        case 3: obj2_prio += adj; break;
        case 4: obj3_prio += adj; break;
        case 5: roz_prio += adj; break;
        case 14: black_out += adj; color_change = adj != 0; break;
        case 15: white_out += adj; color_change = adj != 0; break;
        case 16: dar_flag += adj; dar_change = adj != 0; break;
        default: break;
    }

    if (adj != 0 && sel >= 6 && sel <= 13)
    {
        blend_mode ^= (1 << (sel - 6));
    }

    if (input_pressed(DOWN)) sel++;
    if (input_pressed(UP)) sel--;

    WRAP(sel, 16);
    WRAP(fg_prio, 15);
    WRAP(obj0_prio, 15);
    WRAP(obj1_prio, 15);
    WRAP(obj2_prio, 15);
    WRAP(obj3_prio, 15);
    WRAP(roz_prio, 15);
    WRAP(dar_flag, 1);

    if (color_change)
    {
        set_default_palette();
        set_gradient2(white_out, 255, 255, 255, 255, 255, 255);
        set_gradient2(black_out, 0, 0, 0, 0, 0, 0);
    }

    if (dar_change)
    {
        *(uint16_t *)0x500000 = dar_flag ? 0x0100 : 0x0000;
    }

    tc0360pri_set_roz_prio(roz_prio, roz_prio, roz_prio, roz_prio);
    tc0360pri_set_obj_prio(obj0_prio, obj1_prio, obj2_prio, obj3_prio);
    tc0360pri_set_tile_prio(fg_prio, 0, 0);
    tc0360pri_set_blendmode(blend_mode);
}

PAGE_REGISTER(general_360pri, init_360pri, update_360pri, NULL);

#endif
