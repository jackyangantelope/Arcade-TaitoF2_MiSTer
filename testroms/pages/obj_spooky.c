#include "../system.h"
#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"
#include "../obj_test.h"

void init_obj_spooky()
{
    reset_screen();
}

static uint8_t zoom[6];
static int zoom_idx = 0;

void update_obj_spooky()
{
    if (input_pressed(LEFT)) zoom_idx--;
    if (input_pressed(RIGHT)) zoom_idx++;
    if (zoom_idx < 0) zoom_idx = 5;
    if (zoom_idx > 5) zoom_idx = 0;

    if (input_pressed(UP)) zoom[zoom_idx] += input_down(BTN1) ? 0x10 : 0x01;
    if (input_pressed(DOWN)) zoom[zoom_idx] -= input_down(BTN1) ? 0x10 : 0x01;

    uint16_t cmd_base = OBJCMD_6BPP;
    wait_dma();
    wait_vblank();

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;

    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 100, 30); obj_commit_reset(o, &obj_ptr);
 
    o->zoom_y = zoom[0]; obj_commit_reset(o, &obj_ptr);

    GridOptions opt;
    memset(&opt, 0, sizeof(opt));
    opt.w = 3; opt.h = 3;
    opt.extra = opt.zoom = 0b100'000'000;
    opt.zoom_x = 0x00; opt.zoom_y = 0x00;
    opt.pos = 0;
    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;   
    
    opt.zoom_x = 0x00; opt.zoom_y = zoom[1];
    obj_grid(10, 10, &opt, &obj_ptr);
    
    opt.zoom_x = 0x00; opt.zoom_y = zoom[2];
    obj_grid(100, 10, &opt, &obj_ptr);

    opt.zoom_x = 0x00; opt.zoom_y = zoom[3];
    obj_grid(200, 10, &opt, &obj_ptr);

    opt.latch_y = 0b000'000'000; opt.inc_y = 0b000'000'000;
    opt.latch_x = 0b011'111'111; opt.inc_x = 0b011'111'111;   
    opt.zoom_x = 0x00; opt.zoom_y = zoom[4];
    obj_grid(10, 100, &opt, &obj_ptr);
    (obj_ptr - 3)->zoom_y = zoom[5];
    
    on_layer(FG0);
    pen_color(1);

    print_at(2, 2, "ZOOM: ");
    for( int i = 0; i < 6; i++ )
    {
        pen_color(zoom_idx == i ? 0 : 1);
        print("%02X  ", zoom[i]);
    }
}

PAGE_REGISTER(obj_spooky, init_obj_spooky, update_obj_spooky, NULL);

