
#include "sim_core.h"
#include "sim_ui.h"
#include "sim_hw_ui.h"
#include "sim_video.h"
#include "sim_state.h"
#include "sim_command.h"
#include "sim_hierarchy.h"
#include "games.h"
#include "F2.h"
#include "F2___024root.h"
#include "imgui_wrap.h"
#include "tc0200obj.h"
#include "file_search.h"
#include "sim_sdram.h"
#include "sim_ddr.h"

#include <stdio.h>
#include <iostream>
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
    CommandQueue command_queue;
    std::string game_name_str = "finalb";
    
    // Parse command line arguments
    if (!command_queue.parse_arguments(argc, argv, game_name_str))
    {
        return -1;
    }
    
    const char *game_name = game_name_str.c_str();
    char title[64];

    const game_t game = game_find(game_name);
    if (game == GAME_INVALID)
    {
        printf("Game '%s' is not found.\n", game_name);
        return -1;
    }

    snprintf(title, 64, "F2 - %s", game_name);

    sim_init();
    
    // Only initialize UI if not in headless mode
    if (!command_queue.is_headless())
    {
        ui_init(title);
    }

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

    if (!command_queue.is_headless())
    {
        video.init(320, 224, imgui_get_renderer());
        init_obj_cache(imgui_get_renderer(),
                       ddr_memory.memory.data() + OBJ_DATA_DDR_BASE,
                       top->rootp->F2_SIGNAL(color_ram, ram_l).m_storage,
                       top->rootp->F2_SIGNAL(color_ram, ram_h).m_storage);
    }
    else
    {
        // Minimal init for headless mode
        video.init(320, 224, nullptr);
    }

    Verilated::traceEverOn(true);

    const Uint8 *keyboard_state = command_queue.is_headless() ? nullptr : SDL_GetKeyboardState(NULL);
    bool screenshot_key_pressed = false;
    
    // Execute pre-simulation commands (like load-state)
    while (!command_queue.empty())
    {
        Command& cmd = command_queue.front();
        if (cmd.type == CommandType::LOAD_STATE)
        {
            if (command_queue.is_verbose())
                std::cout << "Loading state from: " << cmd.filename << std::endl;
            state_manager->restore_state(cmd.filename.c_str());
            command_queue.pop();
        }
        else
        {
            break;  // Stop at first non-load command
        }
    }
    
    // Track frame counting for interactive mode
    bool prev_vblank = false;
    
    // Main loop
    bool running = true;
    while (running)
    {
        // Check if we have commands to execute
        if (!command_queue.empty())
        {
            Command& cmd = command_queue.front();
            
            switch (cmd.type)
            {
            case CommandType::RUN_CYCLES:
                if (command_queue.is_verbose())
                    std::cout << "Running " << cmd.count << " cycles..." << std::endl;
                sim_tick(cmd.count);
                if (command_queue.is_verbose())
                    std::cout << "Completed running " << cmd.count << " cycles" << std::endl;
                command_queue.pop();
                break;
                
            case CommandType::RUN_FRAMES:
                if (command_queue.is_verbose())
                    std::cout << "Running " << cmd.count << " frames..." << std::endl;
                for (uint64_t i = 0; i < cmd.count; i++)
                {
                    sim_tick_until([&] { return top->vblank == 0; });
                    sim_tick_until([&] { return top->vblank != 0; });
                }
                if (command_queue.is_verbose())
                    std::cout << "Completed running " << cmd.count << " frames" << std::endl;
                command_queue.pop();
                break;
                
            case CommandType::SCREENSHOT:
                video.update_texture();
                if (command_queue.is_verbose())
                    std::cout << "Saving screenshot to: " << cmd.filename << std::endl;
                if (video.save_screenshot(cmd.filename.c_str()))
                {
                    if (command_queue.is_verbose())
                        std::cout << "Screenshot saved successfully" << std::endl;
                }
                else
                {
                    std::cerr << "Failed to save screenshot" << std::endl;
                }
                command_queue.pop();
                break;
                
            case CommandType::SAVE_STATE:
                if (command_queue.is_verbose())
                    std::cout << "Saving state to: " << cmd.filename << std::endl;
                state_manager->save_state(cmd.filename.c_str());
                command_queue.pop();
                break;
                
            case CommandType::EXIT:
                if (command_queue.is_verbose())
                    std::cout << "Exiting..." << std::endl;
                running = false;
                command_queue.pop();
                break;
                
            default:
                std::cerr << "Unhandled command type" << std::endl;
                command_queue.pop();
                break;
            }
        }
        else if (!command_queue.is_headless())
        {
            // Interactive mode
            if (!ui_begin_frame())
            {
                running = false;
                break;
            }
            
            // Handle F12 screenshot key
            if (keyboard_state && keyboard_state[SDL_SCANCODE_F12] && !screenshot_key_pressed)
            {
                std::string filename = video.generate_screenshot_filename(game_name);
                video.save_screenshot(filename.c_str());
                screenshot_key_pressed = true;
            }
            else if (keyboard_state && !keyboard_state[SDL_SCANCODE_F12])
            {
                screenshot_key_pressed = false;
            }
            
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
        else
        {
            // Headless mode with no commands left
            running = false;
        }
    }

    sim_shutdown();
    video.deinit();

    delete state_manager;
    return 0;
}
