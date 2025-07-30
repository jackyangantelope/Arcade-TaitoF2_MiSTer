#include "imgui_wrap.h"
#include "sim_ui.h"
#include "imgui_wrap.h"
#include "imgui_memory_editor.h"
#include "sim_core.h"
#include "sim_state.h"
#include "games.h"
#include "F2.h"
#include "F2___024root.h"
#include "verilated_fst_c.h"
#include "sim_sdram.h"
#include "sim_ddr.h"

extern VerilatedContext *contextp;
extern F2 *top;
extern std::unique_ptr<VerilatedFstC> tfp;
extern SimState* state_manager;
extern SimSDRAM sdram;
extern SimDDR ddr_memory;
extern uint64_t total_ticks;
extern bool simulation_run;
extern bool simulation_step;
extern int simulation_step_size;
extern bool simulation_step_vblank;
extern bool simulation_wp_set;
extern int simulation_wp_addr;
extern uint64_t simulation_reset_until;
extern bool system_pause;
extern char trace_filename[64];
extern int trace_depth;
extern bool trace_active;

extern uint32_t dipswitch_a;
extern uint32_t dipswitch_b;

#define blockram_16_rw(instance, size) \
ImU8 instance##_read(const ImU8* , size_t off, void*) \
{ \
    size_t word_off = off >> 1; \
    if (off & 1) \
        return top->rootp->F2__DOT__##instance##__DOT__ram_l[word_off]; \
    else \
        return top->rootp->F2__DOT__##instance##__DOT__ram_h[word_off]; \
} \
void instance##_write(ImU8* , size_t off, ImU8 d, void*) \
{ \
    size_t word_off = off >> 1; \
    if (off & 1) \
        top->rootp->F2__DOT__##instance##__DOT__ram_l[word_off] = d; \
    else \
        top->rootp->F2__DOT__##instance##__DOT__ram_h[word_off] = d; \
} \
class instance##_Editor : public MemoryEditor \
{ \
public: \
    instance##_Editor() : MemoryEditor() \
    { \
        ReadFn = instance##_read; \
        WriteFn = instance##_write; \
    } \
    void DrawContents() \
    { \
        MemoryEditor::DrawContents(nullptr, size); \
    } \
}; \
instance##_Editor instance;

blockram_16_rw(scn_ram_0, 64 * 1024);
blockram_16_rw(scn_mux_ram, 64 * 1024);
blockram_16_rw(color_ram, 8 * 1024);
blockram_16_rw(obj_ram, 64 * 1024);
blockram_16_rw(work_ram, 64 * 1024);
blockram_16_rw(pivot_ram, 8 * 1024);

MemoryEditor scn_main_rom;
MemoryEditor rom_mem;
MemoryEditor ddr_mem_editor;
MemoryEditor sound_ram;
MemoryEditor sound_rom;
MemoryEditor extension_ram;

void ui_init(const char *title) {
    imgui_init(title);
}



bool ui_begin_frame() {
    return imgui_begin_frame();
}

void ui_end_frame() {
    imgui_end_frame();
}

void ui_draw() {
    if (ImGui::Begin("Simulation Control"))
    {
        ImGui::LabelText("Ticks", "%llu", total_ticks);
        ImGui::Checkbox("Run", &simulation_run);
        if (ImGui::Button("Step"))
        {
            simulation_step = true;
            simulation_run = false;
        }
        ImGui::InputInt("Step Size", &simulation_step_size);
        ImGui::Checkbox("Step Frame", &simulation_step_vblank);
        
        ImGui::Checkbox("WP Set", &simulation_wp_set);
        ImGui::SameLine();
        ImGui::InputInt("##wpaddr", &simulation_wp_addr, 0, 0,ImGuiInputTextFlags_CharsHexadecimal);

        if (ImGui::Button("Reset"))
        {
            simulation_reset_until = total_ticks + 100;
        }

        ImGui::SameLine();
        ImGui::Checkbox("Pause", &system_pause);


        ImGui::Separator();
        
        // Save/Restore State Section
        ImGui::Text("Save/Restore State");
        
        static char state_filename[256] = "state.f2state";
        ImGui::InputText("State Filename", state_filename, sizeof(state_filename));
        
        static std::vector<std::string> state_files = state_manager->get_f2state_files();
        static int selected_state_file = -1;
        
        // Auto-generate filename when file list is loaded/updated
        static bool first_load = true;
        if (first_load)
        {
            std::string auto_name = state_manager->generate_next_state_name();
            strncpy(state_filename, auto_name.c_str(), sizeof(state_filename) - 1);
            state_filename[sizeof(state_filename) - 1] = '\0';
            first_load = false;
        }
        
        if (ImGui::Button("Save State"))
        {
            // Ensure filename has .f2state extension
            std::string filename = state_filename;
            if (filename.size() < 8 || filename.substr(filename.size() - 8) != ".f2state")
            {
                filename += ".f2state";
                strncpy(state_filename, filename.c_str(), sizeof(state_filename) - 1);
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
                std::string auto_name = state_manager->generate_next_state_name();
                strncpy(state_filename, auto_name.c_str(), sizeof(state_filename) - 1);
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
                if (ImGui::Selectable(state_files[i].c_str(), selected_state_file == (int)i, ImGuiSelectableFlags_AllowDoubleClick))
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
        if(ImGui::InputInt("Trace Depth", &trace_depth, 1, 10,
                           trace_active ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None))
        {
            trace_depth = std::min(std::max(trace_depth, 1), 99);
        }
        ImGui::PopItemWidth();
        ImGui::InputText("Filename", trace_filename, sizeof(trace_filename),
                         tfp ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_None);
        if(ImGui::Button(tfp ? "Stop Tracing###TraceBtn" : "Start Tracing###TraceBtn"))
        {
            if (tfp)
            {
                tfp->close();
                tfp.reset();
            }
            else
            {
                if (strlen(trace_filename) > 0)
                {
                    tfp = std::make_unique<VerilatedFstC>();
                    top->trace(tfp.get(), trace_depth);
                    tfp->open(trace_filename);
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
                extension_ram.DrawContents(top->rootp->F2__DOT__tc0200obj_extender__DOT__extension_ram__DOT__ram.m_storage, 4 * 1024);
                ImGui::EndTabItem();
            }

            
            if (ImGui::BeginTabItem("CPU ROM"))
            {
                rom_mem.DrawContents(sdram.data + CPU_ROM_SDR_BASE, 1024 * 1024);
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
             
            if (ImGui::BeginTabItem("DDR"))
            {
                ddr_mem_editor.DrawContents(ddr_memory.memory.data(), ddr_memory.size);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Sound RAM"))
            {
                sound_ram.DrawContents(top->rootp->F2__DOT__sound_ram__DOT__ram.m_storage, 16 * 1024);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Sound ROM"))
            {
                sound_rom.DrawContents(top->rootp->F2__DOT__sound_rom__DOT__ram.m_storage, 128 * 1024);
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
            for( int i = 0; i < 8; i++ )
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
            for( int i = 0; i < 8; i++ )
            {
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                ImGui::CheckboxFlags("##dwsa", &dipswitch_a, ((uint32_t)1 << i));
                ImGui::PopID();
            }
            ImGui::TableNextColumn();
            ImGui::Text("DWSB");
            for( int i = 0; i < 8; i++ )
            {
                ImGui::TableNextColumn();
                ImGui::PushID(i);
                ImGui::CheckboxFlags("##dwsb", &dipswitch_b, ((uint32_t)1 << i));
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
    }
    ImGui::End();
}