int16_t invalid_read_count;

void init_scn_align()
{
    reset_screen();

    // Max extent corner boundaries
    on_layer(BG0); pen_color(0);
    sym_at(1, 1, 1);
    sym_at(1, 28, 1);
    sym_at(40, 1, 1);
    sym_at(40, 28, 1);

    // Should not be visible
    pen_color(1);
    sym_at(0, 1, 0x1b);
    sym_at(1, 0, 0x1b);
    sym_at(0, 28, 0x1b);
    sym_at(1, 29, 0x1b);
    sym_at(40, 0, 0x1b);
    sym_at(41, 1, 0x1b);
    sym_at(40, 29, 0x1b);
    sym_at(41, 28, 0x1b);

    pen_color(6);
    print_at(4, 3, "LAYER BG0");
    on_layer(BG1);
    pen_color(3);
    print_at(4, 4, "LAYER BG1");
    on_layer(FG0);
    pen_color(0);
    print_at(4, 5, "LAYER FG0");

    on_layer(BG0);
    pen_color(9);
    print_at(10, 8, "NO SCROLL");
    print_at(10, 9, "NO SCROLL");
    print_at(10, 20, "NO SCROLL");
    print_at(10, 21, "NO SCROLL");
    for (int y = 0; y < 10; y++)
    {
        sym_at(8, 10 + y, 0x12);
        print_at(10, 10 + y, "%d SCROLL", y);
    }

#if HAS_TC0100SCN
    for( int y = 0; y < 10 * 8; y++ )
    {
        TC0100SCN->bg0_rowscroll[(8 * 10) + y] = y - 39;
    }
#endif

    frame_count = 0;
    invalid_read_count = 0;
}

void update_scn_align()
{
    wait_vblank();

    bool flip = 0; //(input_dsw() & 0x02) == 0;

    int16_t base_x;
    int16_t base_y;
    uint16_t system_flags;

    if (flip)
    {
        base_x = 7;
        base_y = 16;
        system_flags = TC0100SCN_SYSTEM_FLIP;
    }
    else
    {
        base_x = 9;
        base_y = 0;
        system_flags = 0;
    }

#if HAS_TC0100SCN
    TC0100SCN_Ctrl->bg1_y = base_y;
    TC0100SCN_Ctrl->bg1_x = base_x;
    TC0100SCN_Ctrl->fg0_y = base_y;
    TC0100SCN_Ctrl->fg0_x = base_x;
    TC0100SCN_Ctrl->system_flags = system_flags;
    TC0100SCN_Ctrl->layer_flags = 0;
    TC0100SCN_Ctrl->bg0_y = base_y;
    TC0100SCN_Ctrl->bg0_x = base_x;
#endif

    TC0200OBJ_Inst *obj_ptr = TC0200OBJ;
    TC0200OBJ_Inst work;
    TC0200OBJ_Inst *o = &work;
    uint16_t cmd_base = OBJCMD_6BPP;

    if (flip) cmd_base |= OBJCMD_FLIPSCREEN;

    obj_reset(o);
    obj_cmd(o, cmd_base); obj_commit_reset(o, &obj_ptr);
    obj_master_xy(o, 100, 30); obj_commit_reset(o, &obj_ptr);
  
    GridOptions opt;
    opt.w = 2; opt.h = 2;
    opt.extra = opt.zoom = 0b100'000'000;
    opt.seq = 0b111'111'110;
    opt.latch_y = 0b011'011'011; opt.inc_y = 0b011'011'011;
    opt.latch_x = 0b000'111'111; opt.inc_x = 0b000'100'100;
    opt.zoom_x = 0; opt.zoom_y = 0;
    opt.pos = 0;
    obj_grid(135, 40, &opt, &obj_ptr);

    opt.w = 1; opt.h = 1;
    opt.extra = opt.zoom = 1;
    opt.latch_x = opt.latch_y = 0;
    opt.seq = 0;
    opt.zoom_x = 0; opt.zoom_y = 0;
    opt.pos = 0;
    obj_grid(8, -15, &opt, &obj_ptr);
    obj_grid(8, 200, &opt, &obj_ptr);


    for( int i = 0; i < 512; i++ )
    {
        obj_ptr[i].pos0 = frame_count;
    }
    for( int i = 0; i < 512; i++ )
    {
        if( obj_ptr[i].pos0 != frame_count )
        {
            *(uint32_t *)0xff0000 = 1;
            invalid_read_count++;
        }
    }

    frame_count++;

    on_layer(BG0);
    move_to(20, 1);
    print("%u %u", frame_count, invalid_read_count);
}



uint16_t system_flags, layer_flags;

void init_scn_control_access()
{
    reset_screen();

    frame_count = 0;
    system_flags = 0;
    layer_flags = 0;

    const char *msg1 = "UP/DOWN ADJUST SYSTEM FLAGS";
    const char *msg2 = "LEFT/RIGHT ADJUST LAYER FLAGS";

    pen_color(0);
    on_layer(BG0);
    print_at(4, 20, msg1);
    on_layer(BG1);
    print_at(4, 20, msg1);
    on_layer(FG0);
    print_at(4, 20, msg1);

    on_layer(BG0);
    print_at(4, 21, msg2);
    on_layer(BG1);
    print_at(4, 21, msg2);
    on_layer(FG0);
    print_at(4, 21, msg2);
}

void update_scn_control_access()
{
    bool changed = false;

    if(input_pressed(LEFT))
    {
        if (system_flags == 0)
            system_flags = 1;
        else
            system_flags = system_flags << 1;
        changed = true;
    }

    if(input_pressed(RIGHT))
    {
        if (system_flags == 0)
            system_flags = 1 << 15;
        else
            system_flags = system_flags >> 1;
        changed = true;
    }

    if(input_pressed(UP))
    {
        if (layer_flags == 0)
            layer_flags = 1;
        else
            layer_flags = layer_flags << 1;
        changed = true;
    }

    if(input_pressed(DOWN))
    {
        if (layer_flags == 0)
            layer_flags = 1 << 15;
        else
            layer_flags = layer_flags >> 1;
        changed = true;
    }

#if HAS_TC0100SCN
    TC0100SCN_Ctrl->system_flags = system_flags;
    TC0100SCN_Ctrl->layer_flags = layer_flags;
#endif

    if (changed || frame_count == 0)
    {
        pen_color(0);
        on_layer(BG0);
        print_at(4, 4, "LAYER: %06X", layer_flags);
        on_layer(BG1);
        print_at(4, 5, "LAYER: %06X", layer_flags);
        on_layer(FG0);
        print_at(4, 6, "LAYER: %06X", layer_flags);

        pen_color(0);
        on_layer(BG0);
        print_at(20, 4, "SYSTEM: %06X", system_flags);
        on_layer(BG1);
        print_at(20, 5, "SYSTEM: %06X", system_flags);
        on_layer(FG0);
        print_at(20, 6, "SYSTEM: %06X", system_flags);
    }

    frame_count++;
}


static int fail_index = 0;

void init_romtest()
{
    reset_screen();

    on_layer(FG0);
    pen_color(8);
    print_at(1,1,"ROM TEST");
}

const uint16_t rom_data[256] __attribute__ ((aligned (1024))) =
{
    0x00ff, 0x01fe, 0x02fd, 0x03fc, 0x04fb, 0x05fa, 0x06f9, 0x07f8, 0x08f7, 0x09f6, 0x0af5, 0x0bf4, 0x0cf3, 0x0df2, 0x0ef1, 0x0ff0,
    0x10ef, 0x11ee, 0x12ed, 0x13ec, 0x14eb, 0x15ea, 0x16e9, 0x17e8, 0x18e7, 0x19e6, 0x1ae5, 0x1be4, 0x1ce3, 0x1de2, 0x1ee1, 0x1fe0,
    0x20df, 0x21de, 0x22dd, 0x23dc, 0x24db, 0x25da, 0x26d9, 0x27d8, 0x28d7, 0x29d6, 0x2ad5, 0x2bd4, 0x2cd3, 0x2dd2, 0x2ed1, 0x2fd0,
    0x30cf, 0x31ce, 0x32cd, 0x33cc, 0x34cb, 0x35ca, 0x36c9, 0x37c8, 0x38c7, 0x39c6, 0x3ac5, 0x3bc4, 0x3cc3, 0x3dc2, 0x3ec1, 0x3fc0,
    0x40bf, 0x41be, 0x42bd, 0x43bc, 0x44bb, 0x45ba, 0x46b9, 0x47b8, 0x48b7, 0x49b6, 0x4ab5, 0x4bb4, 0x4cb3, 0x4db2, 0x4eb1, 0x4fb0,
    0x50af, 0x51ae, 0x52ad, 0x53ac, 0x54ab, 0x55aa, 0x56a9, 0x57a8, 0x58a7, 0x59a6, 0x5aa5, 0x5ba4, 0x5ca3, 0x5da2, 0x5ea1, 0x5fa0,
    0x609f, 0x619e, 0x629d, 0x639c, 0x649b, 0x659a, 0x6699, 0x6798, 0x6897, 0x6996, 0x6a95, 0x6b94, 0x6c93, 0x6d92, 0x6e91, 0x6f90,
    0x708f, 0x718e, 0x728d, 0x738c, 0x748b, 0x758a, 0x7689, 0x7788, 0x7887, 0x7986, 0x7a85, 0x7b84, 0x7c83, 0x7d82, 0x7e81, 0x7f80,
    0x807f, 0x817e, 0x827d, 0x837c, 0x847b, 0x857a, 0x8679, 0x8778, 0x8877, 0x8976, 0x8a75, 0x8b74, 0x8c73, 0x8d72, 0x8e71, 0x8f70,
    0x906f, 0x916e, 0x926d, 0x936c, 0x946b, 0x956a, 0x9669, 0x9768, 0x9867, 0x9966, 0x9a65, 0x9b64, 0x9c63, 0x9d62, 0x9e61, 0x9f60,
    0xa05f, 0xa15e, 0xa25d, 0xa35c, 0xa45b, 0xa55a, 0xa659, 0xa758, 0xa857, 0xa956, 0xaa55, 0xab54, 0xac53, 0xad52, 0xae51, 0xaf50,
    0xb04f, 0xb14e, 0xb24d, 0xb34c, 0xb44b, 0xb54a, 0xb649, 0xb748, 0xb847, 0xb946, 0xba45, 0xbb44, 0xbc43, 0xbd42, 0xbe41, 0xbf40,
    0xc03f, 0xc13e, 0xc23d, 0xc33c, 0xc43b, 0xc53a, 0xc639, 0xc738, 0xc837, 0xc936, 0xca35, 0xcb34, 0xcc33, 0xcd32, 0xce31, 0xcf30,
    0xd02f, 0xd12e, 0xd22d, 0xd32c, 0xd42b, 0xd52a, 0xd629, 0xd728, 0xd827, 0xd926, 0xda25, 0xdb24, 0xdc23, 0xdd22, 0xde21, 0xdf20,
    0xe01f, 0xe11e, 0xe21d, 0xe31c, 0xe41b, 0xe51a, 0xe619, 0xe718, 0xe817, 0xe916, 0xea15, 0xeb14, 0xec13, 0xed12, 0xee11, 0xef10,
    0xf00f, 0xf10e, 0xf20d, 0xf30c, 0xf40b, 0xf50a, 0xf609, 0xf708, 0xf807, 0xf906, 0xfa05, 0xfb04, 0xfc03, 0xfd02, 0xfe01, 0xff00,
};

void update_romtest()
{
    on_layer(FG0);
    pen_color(8);
    print_at(20, 1, "%04X", vblank_count);
    
    if (fail_index > 20) return;

    for( uint16_t x = 0; x < 256; x++ )
    {
        uint16_t expected = ( x << 8 ) | (255 - x);
        uint16_t rom_value = rom_data[x];

        if (rom_value != expected)
        {
            print_at(1, 2 + fail_index, "%04X %04X", expected, rom_value);
            fail_index++;
        }
    }
}


