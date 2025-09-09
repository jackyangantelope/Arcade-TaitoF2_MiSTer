#include "../system.h"
#include "../page.h"

#include "../util.h"
#include "../tilemap.h"
#include "../input.h"


void send_sound_code(uint8_t code)
{
    *SYT_ADDR = 0;
    *SYT_DATA = (code >> 4) & 0xf;
    *SYT_ADDR = 1;
    *SYT_DATA = code & 0xf;
    *SYT_ADDR = 2;
    *SYT_DATA = (code >> 4) & 0xf;
    *SYT_ADDR = 3;
    *SYT_DATA = code & 0xf;
}

bool read_sound_response(uint8_t *code)
{
    uint8_t r = 0;
    *SYT_ADDR = 4;

    uint8_t status = *SYT_DATA;

    if (status & 0x4)
    {
        *SYT_ADDR = 0;
        r = (*SYT_DATA << 4);
        *SYT_ADDR = 1;
        r |= *SYT_DATA;

        if (code) *code = r;
        return true;
    }

    return false;
}

static uint8_t sound_code = 0;
static uint8_t sound_msg = 0;
static uint8_t sound_msg2 = 0;

static uint8_t sound_data[16];

void init_sound_test()
{
    reset_screen();
    
    // Reset
    *SYT_ADDR = 4;
    *SYT_DATA = 1;


    *SYT_ADDR = 4;
    *SYT_DATA = 0;


/*    wait_vblank();
    wait_vblank();
    wait_vblank();
    send_sound_code(0xFE);
    wait_vblank();
    wait_vblank();
    wait_vblank();
    wait_vblank();
    send_sound_code(0xA0);
    sound_code = 0xA0;*/
}

void update_sound_test()
{
    wait_vblank();

    if(input_pressed(LEFT))
    {
        sound_code--;
    }

    if(input_pressed(RIGHT))
    {
        sound_code++;
    }

    if(input_pressed(BTN1))
    {
        //send_sound_code(sound_code);
        extern void run_mdfourier();
        run_mdfourier();
    }
        
    *SYT_ADDR = 4;
    uint8_t status = (*SYT_DATA) & 0xf;

    if(status & 0x04)
    {
        *SYT_ADDR = 0;
        sound_msg = (*SYT_DATA) << 4;
        *SYT_ADDR = 1;
        sound_msg |= (*SYT_DATA) & 0x0f;
    }

    if(status & 0x08)
    {
        *SYT_ADDR = 2;
        sound_msg2 = (*SYT_DATA << 4);
        *SYT_ADDR = 3;
        sound_msg2 |= (*SYT_DATA & 0x0f);
    }

    if(input_pressed(BTN3))
    {
        pen_color(1);
        on_layer(FG0);
        move_to(4, 10);
        for( int i = 0; i < 16; i++ )
        {
            *SYT_ADDR = i;
            sound_data[i] = *SYT_DATA;
            print("%01X ", sound_data[i] & 0xf);
        }
    }

    pen_color(0);
    on_layer(FG0);
    print_at(4, 4, "SOUND CODE: %02X", sound_code);
    print_at(4, 5, "STATUS: %02X", status);
    print_at(4, 6, "MESSAGE: %02X %02X", sound_msg, sound_msg2);
}

PAGE_REGISTER(sound_test, init_sound_test, update_sound_test, NULL);

