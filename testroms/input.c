#include "input.h"

#include "system.h"

static uint16_t s_prev = 0;
static uint16_t s_cur = 0;
static uint16_t s_dsw = 0;
static uint16_t s_count = 0;

void input_init()
{
#if HAS_TE7750
    TE7750->p4 = 0;
    TE7750->p5 = 0;
    TE7750->CR0 = 0xff;
    TE7750->CR1 = 0xff;
    TE7750->CR2 = 0xe7;
    TE7750->CR3 = 0xff;
    *(uint16_t *)0x900000 = 0; // ????
#endif
}

void input_update()
{
    if (s_cur == s_prev && s_cur != 0xffff)
    {
        s_count++;
    }
    else
    {
        s_count = 0;
    }

    s_prev = s_cur;
#if HAS_TE7750
    s_cur = TE7750->p6;
    
    s_dsw = ( TE7750->p1 & 0x00ff ) | ( TE7750->p2 << 8 );

#else
    s_cur = TC0220IOC->p1;

    s_dsw = ( TC0220IOC->sw_a & 0x00ff ) | ( TC0220IOC->sw_b << 8 );
#endif

    if (s_count == 5)
    {
        s_cur = 0xffff;
        s_count = 0;
    }
}

uint16_t input_state()
{
    return s_cur;
}

bool input_down(InputKey key)
{
    return (s_cur & key) != key;
}

bool input_released(InputKey key)
{
    return ((s_cur & key) != 0) && (((s_prev ^ s_cur) & key) != 0);
}

bool input_pressed(InputKey key)
{
    return ((s_cur & key) != key) && (((s_prev ^ s_cur) & key) != 0);
}

uint16_t input_dsw()
{
    return s_dsw;
}
