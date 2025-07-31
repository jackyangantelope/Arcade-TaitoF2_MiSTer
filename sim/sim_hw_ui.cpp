
#include "sim_hw_ui.h"
#include "imgui.h"
#include "F2.h"
#include "F2___024root.h"
#include "sim_sdram.h"
#include "dis68k/dis68k.h"
#include "tc0200obj.h"
#include "tc0360pri.h"
#include "tc0480scp.h"

extern F2 *top;
extern SimSDRAM sdram;

#define SWAP32(x)                                                              \
    (((x) & 0xff000000) >> 16) | (((x) & 0x00ff0000) >> 16) |                  \
        (((x) & 0x0000ff00) << 16) | (((x) & 0x000000ff) << 16)

struct SimDebug
{
    uint32_t modified;
    uint32_t zoom;
    uint32_t dy;
    uint32_t y;
};

extern SimDebug *sim_debug_data;

struct PresetZoom
{
    uint16_t y;
    uint8_t zoom;
    uint8_t dy;
};

void hw_ui_draw()
{
    if (ImGui::Begin("Debug"))
    {
        int x = (int16_t)top->rootp->F2__DOT__tc0480scp__DOT__base_xofs;
        int y = (int16_t)top->rootp->F2__DOT__tc0480scp__DOT__base_yofs;
        ImGui::InputInt("Disp X", &x);
        ImGui::InputInt("Disp Y", &y);
        top->rootp->F2__DOT__tc0480scp__DOT__base_xofs = x;
        top->rootp->F2__DOT__tc0480scp__DOT__base_yofs = y;

        int step = 1;
        int step_fast = 16;
        bool modified = false;
        int v = SWAP32(sim_debug_data->y) & 0xffff;
        if (ImGui::InputScalar("BG1 Y", ImGuiDataType_U32, &v, &step,
                               &step_fast, "%04X",
                               ImGuiInputTextFlags_CharsHexadecimal))
        {
            sim_debug_data->y = SWAP32(v);
            modified = true;
        }

        v = SWAP32(sim_debug_data->zoom) & 0xff;
        if (ImGui::InputScalar("BG1 Zoom", ImGuiDataType_U32, &v, &step,
                               &step_fast, "%02X",
                               ImGuiInputTextFlags_CharsHexadecimal))
        {
            sim_debug_data->zoom = SWAP32(v);
            modified = true;
        }

        v = SWAP32(sim_debug_data->dy) & 0xff;
        if (ImGui::InputScalar("BG1 DY", ImGuiDataType_U32, &v, &step,
                               &step_fast, "%02X",
                               ImGuiInputTextFlags_CharsHexadecimal))
        {
            sim_debug_data->dy = SWAP32(v);
            modified = true;
        }

        const char *names[] = { "", "Shrink", "Normal", "Zoom" };
        const PresetZoom presets[IM_ARRAYSIZE(names) - 1] = {
            { 0xfed7, 0x00, 0xff },
            { 0xfed7, 0x00, 0x00 },
            { 0xfed9, 0x00, 0x00 },
        };

        if (ImGui::BeginCombo("Preset", "Select Preset", 0))
        {
            for (int n = 0; n < IM_ARRAYSIZE(presets); n++)
            {
                char label[64];
                snprintf(label, sizeof(label), "%04X,%02X,%02X", presets[n].y, presets[n].zoom, presets[n].dy);
                if (ImGui::Selectable(label, false))
                {
                    sim_debug_data->zoom = SWAP32(presets[n].zoom);
                    sim_debug_data->dy = SWAP32(presets[n].dy);
                    sim_debug_data->y = SWAP32(presets[n].y);
                    modified = true;
                }
            }
            ImGui::EndCombo();
        }

        if (modified)
        {
            sim_debug_data->modified++;
            top->rootp->F2__DOT__rom_cache__DOT__version++;
        }
    }
    ImGui::End();

    draw_obj_window();
    draw_obj_preview_window();
    draw_pri_window();
    draw_480scp_window();

    if (ImGui::Begin("68000"))
    {
        uint32_t pc =
            top->rootp->F2__DOT__m68000__DOT__excUnit__DOT__PcL |
            (top->rootp->F2__DOT__m68000__DOT__excUnit__DOT__PcH << 16);
        ImGui::LabelText("PC", "%08X", pc);
        Dis68k dis(sdram.data + pc, sdram.data + pc + 64, pc);
        char optxt[128];
        uint32_t addr;
        dis.disasm(&addr, optxt, sizeof(optxt));
        ImGui::TextUnformatted(optxt);
    }
    ImGui::End();
}
