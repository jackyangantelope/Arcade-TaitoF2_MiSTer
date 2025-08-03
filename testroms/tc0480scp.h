#if !defined(TC0480SCP_H)
#define TC0480SCP_H 1

#include <stdint.h>

typedef struct TC0480SCP_BG
{
    union
    {
        struct
        {
            uint8_t attr;
            uint8_t color;
        };
        uint16_t attr_color;
    };
    uint16_t code;
} TC0480SCP_BG;

typedef struct TC0480SCP_FG
{
    union
    {
        struct
        {
            uint8_t attr;
            uint8_t code;
        };
        uint16_t attr_code;
    };
} TC0480SCP_FG;


typedef struct TC0480SCP_Layout
{
    TC0480SCP_BG bg0[0x400]; // 0x0000
    TC0480SCP_BG bg1[0x400]; // 0x1000
    TC0480SCP_BG bg2[0x400]; // 0x2000
    TC0480SCP_BG bg3[0x400]; // 0x3000
    uint16_t bg0_rowscroll[0x200]; // 0x4000
    uint16_t bg1_rowscroll[0x200]; // 0x4400
    uint16_t bg2_rowscroll[0x200]; // 0x4800
    uint16_t bg3_rowscroll[0x200]; // 0x4c00
    uint16_t bg0_finescroll[0x200]; // 0x5000
    uint16_t bg1_finescroll[0x200]; // 0x5400
    uint16_t bg2_finescroll[0x200]; // 0x5800
    uint16_t bg3_finescroll[0x200]; // 0x5c00
    uint16_t bg2_zoom[0x200]; // 0x6000
    uint16_t bg3_zoom[0x200]; // 0x6400
    uint16_t bg2_colscroll[0x200]; // 0x6800
    uint16_t bg3_colscroll[0x200]; // 0x6c00
    uint16_t unknown[0x800 * 5]; // 0x7000
    TC0480SCP_FG fg0[0x1000]; // 0xc000
    uint32_t fg0_gfx[0x800]; // 0xe000
} TC0480SCP_Layout;

#define TC0480SCP_SYSTEM_BG2_ZOOM   ((uint16_t)(1 << 0))
#define TC0480SCP_SYSTEM_BG3_ZOOM   ((uint16_t)(1 << 1))
#define TC0480SCP_SYSTEM_PRIO0      ((uint16_t)(1 << 2))
#define TC0480SCP_SYSTEM_PRIO1      ((uint16_t)(1 << 3))
#define TC0480SCP_SYSTEM_PRIO2      ((uint16_t)(1 << 4))
#define TC0480SCP_SYSTEM_EXT_SYNC   ((uint16_t)(1 << 5))
#define TC0480SCP_SYSTEM_FLIP       ((uint16_t)(1 << 6))
#define TC0480SCP_SYSTEM_WIDE       ((uint16_t)(1 << 7))

typedef struct TC0480SCP_Control
{
    uint16_t bg0_x;
    uint16_t bg1_x;
    uint16_t bg2_x;
    uint16_t bg3_x;

    uint16_t bg0_y;
    uint16_t bg1_y;
    uint16_t bg2_y;
    uint16_t bg3_y;
    
    uint16_t bg0_zoom;
    uint16_t bg1_zoom;
    uint16_t bg2_zoom;
    uint16_t bg3_zoom;

    uint16_t fg0_x;
    uint16_t fg0_y;
    
    uint16_t unk1;

    uint16_t system_flags;

    uint16_t bg0_dx;
    uint16_t bg1_dx;
    uint16_t bg2_dx;
    uint16_t bg3_dx;

    uint16_t bg0_dy;
    uint16_t bg1_dy;
    uint16_t bg2_dy;
    uint16_t bg3_dy;

    uint16_t bg0_dx1;
    uint16_t bg1_dx1;
    uint16_t bg2_dx1;
    uint16_t bg3_dx1;

    uint16_t bg0_dy1;
    uint16_t bg1_dy1;
    uint16_t bg2_dy1;
    uint16_t bg3_dy1;


} TC0480SCP_Control;

_Static_assert(sizeof(TC0480SCP_Control) == 0x40, "TC0480SCP_Control mismatch");
_Static_assert(sizeof(TC0480SCP_Layout) == 0x10000, "TC0480SCP_Layout mismatch");

#endif
