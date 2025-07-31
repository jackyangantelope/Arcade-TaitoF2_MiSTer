
#include "sim_core.h"
#include "sim_ui.h"
#include "sim_hw_ui.h"
#include "sim_video.h"
#include "sim_state.h"
#include "games.h"
#include "F2.h"
#include "F2___024root.h"
#include "imgui_wrap.h"
#include "tc0200obj.h"
#include "file_search.h"
#include "sim_sdram.h"
#include "sim_ddr.h"

#include <stdio.h>
#include <SDL.h>

extern SimVideo video;
extern SimDDR ddr_memory;
extern SimSDRAM sdram;
extern F2 *top;

SimState *state_manager = nullptr;

uint32_t dipswitch_a = 0;
uint32_t dipswitch_b = 0;

extern bool simulation_run;
extern bool simulation_step;
extern bool simulation_step_vblank;
extern int simulation_step_size;
extern bool system_pause;

struct SimDebug
{
    uint32_t modified;
    uint32_t zoom;
    uint32_t dy;
    uint32_t y;
};

SimDebug *sim_debug_data = nullptr;

int main(int argc, char **argv)
{
    const char *game_name = "finalb";
    char title[64];

    if (argc == 2)
    {
        game_name = argv[1];
    }

    const game_t game = game_find(game_name);
    if (game == GAME_INVALID)
    {
        printf("Game '%s' is not found.\n", game_name);
        return -1;
    }

    snprintf(title, 64, "F2 - %s", game_name);

    sim_init();
    ui_init(title);

    game_init(game);

    g_fs.addSearchPath(".");

    sim_debug_data = (SimDebug *)(sdram.data + 0x80000);

    top->ss_do_save = 0;
    top->ss_do_restore = 0;
    top->obj_debug_idx = -1;

    top->joystick_p1 = 0;
    top->joystick_p2 = 0;

    state_manager = new SimState(top, &ddr_memory, 0, 512 * 1024);
    state_manager->set_game_name(::game_name(game));

    video.init(320, 224, imgui_get_renderer());

    init_obj_cache(imgui_get_renderer(),
                   ddr_memory.memory.data() + OBJ_DATA_DDR_BASE,
                   top->rootp->F2__DOT__color_ram__DOT__ram_l.m_storage,
                   top->rootp->F2__DOT__color_ram__DOT__ram_h.m_storage);

    Verilated::traceEverOn(true);

    while (ui_begin_frame())
    {
        prune_obj_cache();

        top->dswa = dipswitch_a & 0xff;
        top->dswb = dipswitch_b & 0xff;
        top->pause = system_pause;

        top->joystick_p1 = imgui_get_buttons();

        if (simulation_run || simulation_step)
        {
            if (simulation_step_vblank)
            {
                sim_tick_until([&] { return top->vblank == 0; });
                sim_tick_until([&] { return top->vblank != 0; });
            }
            else
            {
                sim_tick(simulation_step_size);
            }
            video.update_texture();
        }
        simulation_step = false;

        ui_draw();
        hw_ui_draw();
        video.draw();

        ui_end_frame();
    }

    sim_shutdown();
    video.deinit();

    delete state_manager;
    return 0;
}
