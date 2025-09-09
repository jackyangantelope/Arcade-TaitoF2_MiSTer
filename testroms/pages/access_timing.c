#include "../system.h"

#include "../page.h"
#include "../util.h"
#include "../input.h"
#include "../tilemap.h"

void init_access_timing()
{
    reset_screen();
}

void update_access_timing()
{
    uint32_t increment = 0;
    uint32_t scn_write = 0;
    uint32_t obj_write = 0;
    uint32_t dar_write = 0;
    uint32_t roz_write = 0;

#if HAS_TC0480SCP
    volatile uint16_t *scn_addr = &TC0480SCP->bg0[0].code;
#else
    volatile uint16_t *scn_addr = &TC0100SCN->bg0[0].code;
#endif    
    volatile uint16_t *obj_addr = &TC0200OBJ[64].pos0;

#if HAS_TC0260DAR
    volatile uint8_t *dar_addr = (uint8_t *)&TC0260DAR[128];
#endif //HAS_TC0260DAR

#if HAS_TC0430GRW
    volatile uint16_t *roz_addr = &TC0430GRW[128];
#endif

    uint32_t vb = vblank_count;
    while( vb == vblank_count ) {}
    vb = vblank_count;
    
    while( vb == vblank_count )
    {
        increment++;
    }
    vb = vblank_count;

    while( vb == vblank_count )
    {
        *scn_addr = 0x0000;
        scn_write++;
    }
    vb = vblank_count;

    while( vb == vblank_count )
    {
        *obj_addr = 0x0000;
        obj_write++;
    }
    vb = vblank_count;

#if HAS_TC0260DAR
    while( vb == vblank_count )
    {
        *dar_addr = 0x0000;
        dar_write++;
    }
    vb = vblank_count;
#endif

#if HAS_TC0430GRW
    while( vb == vblank_count )
    {
        *roz_addr = 0x0000;
        roz_write++;
    }
    vb = vblank_count;
#endif 

    pen_color(0);
    on_layer(FG0);
    move_to(4, 4);
    print("INCREMENT: %08X\n", increment);
    print("SCN_WRITE: %08X\n", scn_write);
    print("OBJ_WRITE: %08X\n", obj_write);
    print("DAR_WRITE: %08X\n", dar_write);
    print("ROZ_WRITE: %08X\n", roz_write);
}

PAGE_REGISTER(access_timing, init_access_timing, update_access_timing, NULL);

