#include "imgui_wrap.h"
#include "sim_ui.h"
#include "imgui_wrap.h"
#include "third_party/imgui_memory_editor.h"
#include "sim_core.h"
#include "sim_state.h"
#include "sim_hierarchy.h"
#include "sim_command.h"
#include "games.h"
#include "F2.h"
#include "F2___024root.h"
#include "verilated_fst_c.h"
#include "sim_sdram.h"
#include "sim_ddr.h"

extern SimState *state_manager;

static CommandQueue* g_command_queue = nullptr;

extern uint32_t dipswitch_a;
extern uint32_t dipswitch_b;

#define blockram_16_rw(instance, size)                                         \
    ImU8 instance##_read(const ImU8 *, size_t off, void *)                     \
    {                                                                          \
        size_t word_off = off >> 1;                                            \
        if (off & 1)                                                           \
            return g_sim_core.top->rootp->F2_SIGNAL(instance, ram_l)[word_off];            \
        else                                                                   \
            return g_sim_core.top->rootp->F2_SIGNAL(instance, ram_h)[word_off];            \
    }                                                                          \
    void instance##_write(ImU8 *, size_t off, ImU8 d, void *)                  \
    {                                                                          \
        size_t word_off = off >> 1;                                            \
        if (off & 1)                                                           \
            g_sim_core.top->rootp->F2_SIGNAL(instance, ram_l)[word_off] = d;               \
        else                                                                   \
            g_sim_core.top->rootp->F2_SIGNAL(instance, ram_h)[word_off] = d;               \
    }                                                                          \
    class instance##_Editor : public MemoryEditor                              \
    {                                                                          \
      public:                                                                  \
        instance##_Editor() : MemoryEditor()                                   \
        {                                                                      \
            ReadFn = instance##_read;                                          \
            WriteFn = instance##_write;                                        \
        }                                                                      \
        void DrawContents()                                                    \
        {                                                                      \
            MemoryEditor::DrawContents(nullptr, size);                         \
        }                                                                      \
    };                                                                         \
    instance##_Editor instance;

blockram_16_rw(scn_ram_0, 64 * 1024);
blockram_16_rw(scn_mux_ram, 64 * 1024);
blockram_16_rw(color_ram, 8 * 1024);
blockram_16_rw(obj_ram, 64 * 1024);
blockram_16_rw(work_ram, 64 * 1024);
blockram_16_rw(pivot_ram, 8 * 1024);

MemoryEditor scn_main_rom;
MemoryEditor rom_mem;
MemoryEditor sound_ram;
MemoryEditor sound_rom;
MemoryEditor extension_ram;

void ui_init(const char *title)
{
    imgui_init(title);
}

void ui_set_command_queue(CommandQueue* queue)
{
    g_command_queue = queue;
}

bool ui_begin_frame()
{
    return imgui_begin_frame();
}

void ui_end_frame()
{
    imgui_end_frame();
}

static bool refresh_state_files = true;

void ui_game_changed()
{
    char title[64];
    const char *name = g_sim_core.GetGameName();
    
    snprintf(title, sizeof(title), "F2 - %s", name);
    imgui_set_title(title);
    refresh_state_files = true;
}

void ui_draw()
{
    if (ImGui::Begin("Simulation Control"))
    {
        ImGui::LabelText("Ticks", "%llu", g_sim_core.m_total_ticks);
        ImGui::Checkbox("Run", &g_sim_core.m_simulation_run);
        if (ImGui::Button("Step"))
        {
            g_sim_core.m_simulation_step = true;
            g_sim_core.m_simulation_run = false;
        }
        ImGui::InputInt("Step Size", &g_sim_core.m_simulation_step_size);
        ImGui::Checkbox("Step Frame", &g_sim_core.m_simulation_step_vblank);

        ImGui::Checkbox("WP Set", &g_sim_core.m_simulation_wp_set);
        ImGui::SameLine();
        ImGui::InputInt("##wpaddr", &g_sim_core.m_simulation_wp_addr, 0, 0,
                        ImGuiInputTextFlags_CharsHexadecimal);

        if (ImGui::Button("Reset"))
        {
            g_command_queue->add(Command(CommandType::RESET, 100));
        }

        ImGui::SameLine();
        ImGui::Checkbox("Pause", &g_sim_core.m_system_pause);

        ImGui::Separator();

        // Save/Restore State Section
        ImGui::Text("Save/Restore State");

        static char state_filename[256] = "state.f2state";
        ImGui::InputText("State Filename", state_filename,
                         sizeof(state_filename));

        static std::vector<std::string> state_files;
        static int selected_state_file = -1;

        // Auto-generate filename when file list is loaded/updated
        if (refresh_state_files)
        {
            state_files = state_manager->get_f2state_files();
            std::string auto_name = state_manager->generate_next_state_name();
            strncpy(state_filename, auto_name.c_str(),
                    sizeof(state_filename) - 1);
            state_filename[sizeof(state_filename) - 1] = '\0';
            refresh_state_files = false;
        }

        if (ImGui::Button("Save State"))
        {
            // Ensure filename has .f2state extension
            std::string filename = state_filename;
            if (filename.size() < 8 ||
                filename.substr(filename.size() - 8) != ".f2state")
            {
                filename += ".f2state";
                strncpy(state_filename, filename.c_str(),
                        sizeof(state_filename) - 1);
                state_filename[sizeof(state_filename) - 1] = '\0';
            }

            if (state_manager->save_state(state_filename))
            {
                // Update file list after successfully saving
                state_files = state_manager->get_f2state_files();
                // Try to select the newly saved file
                for (size_t i = 0; i < state_files.size(); i++)
                {
                    if (state_files[i] == state_filename)
                    {
                        selected_state_file = i;
                        break;
                    }
                }
                // Auto-generate next filename after successful save
                std::string auto_name =
                    state_manager->generate_next_state_name();
                strncpy(state_filename, auto_name.c_str(),
                        sizeof(state_filename) - 1);
                state_filename[sizeof(state_filename) - 1] = '\0';
            }
        }

        // Show list of state files
        if (state_files.size() > 0)
        {
            ImGui::Text("Available State Files:");
            ImGui::BeginChild("StateFiles", ImVec2(0, 100), true);
            for (size_t i = 0; i < state_files.size(); i++)
            {
                if (ImGui::Selectable(state_files[i].c_str(),
                                      selected_state_file == (int)i,
                                      ImGuiSelectableFlags_AllowDoubleClick))
                {
                    selected_state_file = (int)i;
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        state_manager->restore_state(state_files[i].c_str());
                    }
                }
            }
            ImGui::EndChild();
        }
        else
        {
            ImGui::Text("No state files found (*.f2state)");
        }

        ImGui::Separator();

        ImGui::PushItemWidth(100);
        if (ImGui::InputInt("Trace Depth", &g_sim_core.m_trace_depth, 1, 10,
                            g_sim_core.m_trace_active ? ImGuiInputTextFlags_ReadOnly
                                         : ImGuiInputTextFlags_None))
        {
            g_sim_core.m_trace_depth = std::min(std::max(g_sim_core.m_trace_depth, 1), 99);
        }
        ImGui::PopItemWidth();
        ImGui::InputText("Filename", g_sim_core.m_trace_filename, sizeof(g_sim_core.m_trace_filename),
                         g_sim_core.IsTraceActive() ? ImGuiInputTextFlags_ReadOnly
                             : ImGuiInputTextFlags_None);
        if (ImGui::Button(g_sim_core.IsTraceActive() ? "Stop Tracing###TraceBtn"
                              : "Start Tracing###TraceBtn"))
        {
            if (g_sim_core.IsTraceActive())
            {
                g_sim_core.StopTrace();
            }
            else
            {
                if (strlen(g_sim_core.m_trace_filename) > 0)
                {
                    g_sim_core.StartTrace(g_sim_core.m_trace_filename, g_sim_core.m_trace_depth);
                }
            }
        }
    }

    ImGui::End();

    if (ImGui::Begin("Memory"))
    {
        if (ImGui::BeginTabBar("memory_tabs"))
        {
            if (ImGui::BeginTabItem("Screen RAM"))
            {
                scn_ram_0.DrawContents();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Screen MUX RAM"))
            {
                scn_mux_ram.DrawContents();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Color RAM"))
            {
                color_ram.DrawContents();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("OBJ RAM"))
            {
                obj_ram.DrawContents();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Extension RAM"))
            {
                extension_ram.DrawContents(
                    g_sim_core.top->rootp->F2_SIGNAL(tc0200obj_extender, extension_ram, ram).m_storage,
                    4 * 1024);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("CPU ROM"))
            {
                rom_mem.DrawContents(g_sim_core.sdram->data + CPU_ROM_SDR_BASE,
                                     1024 * 1024);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Work RAM"))
            {
                work_ram.DrawContents();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Pivot RAM"))
            {
                pivot_ram.DrawContents();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Sound RAM"))
            {
                sound_ram.DrawContents(
                    g_sim_core.top->rootp->F2_SIGNAL(sound_ram, ram).m_storage,
                    16 * 1024);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Sound ROM"))
            {
                sound_rom.DrawContents(
                    g_sim_core.top->rootp->F2_SIGNAL(sound_rom, ram).m_storage,
                    128 * 1024);
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    if (ImGui::Begin("Input"))
    {
        if (ImGui::BeginTable("dipswitches", 9))
        {
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
            for (int i = 0; i < 8; i++)
            {
                char n[2];
                n[0] = '0' + i;
                n[1] = 0;
                ImGui::TableSetupColumn(n, ImGuiTableColumnFlags_WidthFixed);
            }
            ImGui::TableHeadersRow();
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("DWSA");
            for (int i = 0; i < 8; i++)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                ImGui::CheckboxFlags("##dwsa", &dipswitch_a,
                                     ((uint32_t)1 << i));
                ImGui::PopID();
            }
            ImGui::TableNextColumn();
            ImGui::Text("DWSB");
            for (int i = 0; i < 8; i++)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                ImGui::CheckboxFlags("##dwsb", &dipswitch_b,
                                     ((uint32_t)1 << i));
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}
