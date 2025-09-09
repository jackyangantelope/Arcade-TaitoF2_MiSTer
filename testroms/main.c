#include <stdint.h>
#include <stdbool.h>
#include "printf/printf.h"

#include "tc0100scn.h"
#include "tc0200obj.h"
#include "tc0480scp.h"
#include "util.h"
#include "interrupts.h"
#include "tilemap.h"
#include "input.h"
#include "system.h"
#include "obj_test.h"
#include "color.h"
#include "page.h"
#include "crc16.h"

volatile uint32_t vblank_count = 0;
volatile uint32_t dma_count = 0;

void wait_vblank()
{
    uint32_t current = vblank_count;
    while( current == vblank_count )
    {
    }
}

void wait_dma()
{
    uint32_t current = dma_count;
    while( current == dma_count )
    {
    }
}


void level5_handler()
{
    vblank_count++;
#if HAS_TC0360PRI
    tc0360pri_vblank();
#endif

#if HAS_TC0220IOC
    TC0220IOC->watchdog = 0;
#endif

#if GAME_DEADCONX
    *(volatile uint16_t *)0x800000 = 0;
#endif
}

void level6_handler()
{
    dma_count++;
}

void illegal_instruction_handler()
{
    disable_interrupts();

    on_layer(FG0);
    pen_color(1);
    print_at(10,10,"ILLEGAL INSTRUCTION");

    while(true) {};
}

int main(int argc, char *argv[])
{
    enable_interrupts();

    input_init();

    input_update();

    page_set_next_active();
    
    while(1)
    {
        input_update();

        // 0x0200 // ACC MODE
        // 0x0100 // Brightness? 
        // 0x0002 // Z4
        // 0x0001 // Z3
        //
        //*(uint16_t*)0x500000 = 0x0100;

        if (input_pressed(START))
        {
            page_set_next_active();
        }

        page_update();
    }

    return 0;
}
