#include <stdint.h>
#include <stddef.h>

#include "interrupts.h"

/* These are defined in the linker script */

extern uint16_t _stext;
extern uint16_t _etext;
extern uint16_t _sbss;
extern uint16_t _ebss;
extern uint16_t _sdata;
extern uint16_t _edata;
extern uint16_t _sstack;
extern uint16_t _estack;

typedef void (*ctor_func_ptr)(void);
extern ctor_func_ptr _sctors[0], _ectors[0];

/* Forward define main */
int main(void);

static void reset_handler(void)
{
    /*__asm__(
        "ori     #0x700, %SR\n\t"
        "move.b  #0, (0x700007)\n\t"
        "move.w  #0, (0x800000)\n\t"
        "lea     (0x700000), %A0\n\t"
        "move.w  #0, (0x6,%A0)\n\t"
        "move.w  #0, (0x8,%A0)\n\t"
        "move.w  #0xff, (0x12,%A0)\n\t"
        "move.w  #0xff, (0x14,%A0)\n\t"
        "move.w  #0xe7, (0x16,%A0)\n\t"
        "move.w  #0xff, (0x18,%A0)\n\t"
        "move.w  #0, (0x900000)\n\t"
        "move.w  %D0, (0x500002)\n\t"
        "lea     (0x700000), %A0\n\t"
        "move.w  #0, (0x6,%A0)\n\t"
        "move.w  #0, (0x8,%A0)\n\t"
        "move.w  #0xff, (0x12,%A0)\n\t"
        "move.w  #0xff, (0x14,%A0)\n\t"
        "move.w  #0xe7, (0x16,%A0)\n\t"
        "move.w  #0xff, (0x18,%A0)\n\t"
        "move.w  #0, (0x900000)\n\t"
        "move.w  %D0, (0x500002)\n\t"
     );

#if GAME_DEADCONX
    //*(volatile uint16_t *)0x800000 = 0;
    //*(volatile uint8_t *)0x700007 = 0;
#endif

*/
    
    /* Copy init values from text to data */
    uint16_t *init_values_ptr = &_etext;
    uint16_t *data_ptr = &_sdata;

    if (init_values_ptr != data_ptr)
    {
        for (; data_ptr < &_edata;)
        {
            *data_ptr++ = *init_values_ptr++;
        }
    }

    /* Clear the zero segment */
    for (uint16_t *bss_ptr = &_sbss; bss_ptr < &_ebss;)
    {
        *bss_ptr++ = 0;
    }

    ctor_func_ptr *ctor_func;
    for (ctor_func = _sctors; ctor_func != _ectors; ctor_func++)
    {
        (*ctor_func)();
    }

#if GAME_DEADCONX
    *(volatile uint16_t *)0x800000 = 0;
#endif

    /* Branch to main function */
    main();

    /* Infinite loop */
    while (1);
}

__attribute__ ((section(".vectors"))) __attribute__((used))
static const void *exception_vectors[256] =
{
    &_estack,
    reset_handler,
    bus_error_handler,
    address_error_handler,
    illegal_instruction_handler,
    zero_divide_handler,
    chk_handler,
    trapv_handler,
    priv_violation_handler,
    trace_handler,
    trap_1010_handler,
    trap_1111_handler,
    NULL,
    NULL,
    NULL,
    uninitialized_handler,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    spurious_handler,
    level1_handler,
    level2_handler,
    level3_handler,
    level4_handler,
    level5_handler,
    level6_handler,
    level7_handler,
    trap0_handler,
    trap1_handler,
    trap2_handler,
    trap3_handler,
    trap4_handler,
    trap5_handler,
    trap6_handler,
    trap7_handler,
    trap8_handler,
    trap9_handler,
    trap10_handler,
    trap11_handler,
    trap12_handler,
    trap13_handler,
    trap14_handler,
    trap15_handler,
    NULL
};

void putchar_(char c) {}
