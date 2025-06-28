#if !defined( TE7750_H )
#define TE7750_H 1

#include <stdint.h>

typedef volatile struct TE7750_Control
{
    uint8_t _pad1;
    uint8_t p1;

    uint8_t _pad2;
    uint8_t p2;

    uint8_t _pad3;
    uint8_t p3;

    uint8_t _pad4;
    uint8_t p4;

    uint8_t _pad5;
    uint8_t p5;

    uint8_t _pad6;
    uint8_t p6;
    
    uint8_t _pad7;
    uint8_t p7;

    uint8_t _pad8;
    uint8_t p8;

    uint8_t _pad9;
    uint8_t p9;
    
    uint8_t _pada;
    uint8_t CR0;

    uint8_t _padb;
    uint8_t CR1;

    uint8_t _padc;
    uint8_t CR2;

    uint8_t _padd;
    uint8_t CR3;
} TE7750_Control;

_Static_assert(sizeof(TE7750_Control) == 26, "TE7750_Control mismatch");

#endif
