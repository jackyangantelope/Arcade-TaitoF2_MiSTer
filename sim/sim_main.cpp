
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
#include "verilated_fst_c.h"

#include <stdio.h>
#include <memory>
#include <SDL.h>

// Access via global SimCore instance

SimState *state_manager = nullptr;

uint32_t dipswitch_a = 0;
uint32_t dipswitch_b = 0;

// Access simulation state via global SimCore instance

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
    bool use_mra = false;
    game_t game = GAME_INVALID;

    // Check if it's an MRA file
    if (game_name_str.length() > 4 && 
        game_name_str.substr(game_name_str.length() - 4) == ".mra") {
        use_mra = true;
        snprintf(title, 64, "F2 - MRA");
    } else {
        game = game_find(game_name);
        if (game == GAME_INVALID)
        {
            printf("Game '%s' is not found.\n", game_name);
            return -1;
        }
        snprintf(title, 64, "F2 - %s", game_name);
    }

    g_sim_core.Init();
    
    // Only initialize UI if not in headless mode
    if (!command_queue.is_headless())
    {
        ui_init(title);
    }

    // Set the game parameter on the core
    if (use_mra) {
        // For MRA mode, set game to a default value (will be overridden by MRA data)
        g_sim_core.top->game = GAME_FINALB;  // Default, ROM loader will determine actual config
        if (!game_init_mra(game_name)) {
            printf("Failed to initialize from MRA file: %s\n", game_name);
            return -1;
        }
    } else {
        g_sim_core.top->game = game;
        game_init(game);
    }

    g_fs.addSearchPath(".");

    sim_debug_data = (SimDebug *)(g_sim_core.sdram->data + 0x80000);

    g_sim_core.top->ss_do_save = 0;
    g_sim_core.top->ss_do_restore = 0;
    g_sim_core.top->obj_debug_idx = -1;

    g_sim_core.top->joystick_p1 = 0;
    g_sim_core.top->joystick_p2 = 0;

    state_manager = new SimState(g_sim_core.top, g_sim_core.ddr_memory, 0, 512 * 1024);
    state_manager->set_game_name(::game_name(game));

    if (!command_queue.is_headless())
    {
        g_sim_core.video->init(320, 224, imgui_get_renderer());
        init_obj_cache(imgui_get_renderer(),
                       g_sim_core.ddr_memory->memory.data() + OBJ_DATA_DDR_BASE,
                       g_sim_core.top->rootp->F2_SIGNAL(color_ram, ram_l).m_storage,
                       g_sim_core.top->rootp->F2_SIGNAL(color_ram, ram_h).m_storage);
    }
    else
    {
        // Minimal init for headless mode
        g_sim_core.video->init(320, 224, nullptr);
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
                printf("Loading state from: %s\n", cmd.filename.c_str());
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
                    printf("Running %llu cycles...\n", cmd.count);
                g_sim_core.Tick(cmd.count);
                if (command_queue.is_verbose())
                    printf("Completed running %llu cycles\n", cmd.count);
                command_queue.pop();
                break;
                
            case CommandType::RUN_FRAMES:
                if (command_queue.is_verbose())
                    printf("Running %llu frames...\n", cmd.count);
                for (uint64_t i = 0; i < cmd.count; i++)
                {
                    g_sim_core.TickUntil([&] { return g_sim_core.top->vblank == 0; });
                    g_sim_core.TickUntil([&] { return g_sim_core.top->vblank != 0; });
                }
                if (command_queue.is_verbose())
                    printf("Completed running %llu frames\n", cmd.count);
                command_queue.pop();
                break;
                
            case CommandType::SCREENSHOT:
                g_sim_core.video->update_texture();
                if (command_queue.is_verbose())
                    printf("Saving screenshot to: %s\n", cmd.filename.c_str());
                if (g_sim_core.video->save_screenshot(cmd.filename.c_str()))
                {
                    if (command_queue.is_verbose())
                        printf("Screenshot saved successfully\n");
                }
                else
                {
                    printf("Failed to save screenshot\n");
                }
                command_queue.pop();
                break;
                
            case CommandType::SAVE_STATE:
                if (command_queue.is_verbose())
                    printf("Saving state to: %s\n", cmd.filename.c_str());
                state_manager->save_state(cmd.filename.c_str());
                command_queue.pop();
                break;
                
            case CommandType::TRACE_START:
                if (command_queue.is_verbose())
                    printf("Starting trace to: %s\n", cmd.filename.c_str());
                g_sim_core.StartTrace(cmd.filename.c_str(), 99);
                if (command_queue.is_verbose())
                    printf("Trace started successfully\n");
                command_queue.pop();
                break;
                
            case CommandType::TRACE_STOP:
                if (command_queue.is_verbose())
                    printf("Stopping trace\n");
                if (g_sim_core.IsTraceActive())
                {
                    g_sim_core.StopTrace();
                    if (command_queue.is_verbose())
                        printf("Trace stopped successfully\n");
                }
                else
                {
                    if (command_queue.is_verbose())
                        printf("No trace was active\n");
                }
                command_queue.pop();
                break;
                
            case CommandType::EXIT:
                if (command_queue.is_verbose())
                    printf("Exiting...\n");
                running = false;
                command_queue.pop();
                break;
                
            default:
                printf("Unhandled command type\n");
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
                std::string filename = g_sim_core.video->generate_screenshot_filename(game_name);
                g_sim_core.video->save_screenshot(filename.c_str());
                screenshot_key_pressed = true;
            }
            else if (keyboard_state && !keyboard_state[SDL_SCANCODE_F12])
            {
                screenshot_key_pressed = false;
            }
            
            prune_obj_cache();

            g_sim_core.top->dswa = dipswitch_a & 0xff;
            g_sim_core.top->dswb = dipswitch_b & 0xff;
            g_sim_core.top->pause = g_sim_core.m_system_pause;

            g_sim_core.top->joystick_p1 = imgui_get_buttons();

            if (g_sim_core.m_simulation_run || g_sim_core.m_simulation_step)
            {
                if (g_sim_core.m_simulation_step_vblank)
                {
                    g_sim_core.TickUntil([&] { return g_sim_core.top->vblank == 0; });
                    g_sim_core.TickUntil([&] { return g_sim_core.top->vblank != 0; });
                }
                else
                {
                    g_sim_core.Tick(g_sim_core.m_simulation_step_size);
                }
                g_sim_core.video->update_texture();
            }
            g_sim_core.m_simulation_step = false;

            ui_draw();
            hw_ui_draw();
            g_sim_core.video->draw();

            ui_end_frame();
        }
        else
        {
            // Headless mode with no commands left
            running = false;
        }
    }

    g_sim_core.Shutdown();
    g_sim_core.video->deinit();

    delete state_manager;
    return 0;
}
