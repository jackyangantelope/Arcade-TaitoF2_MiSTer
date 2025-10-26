#include <SDL.h>

#include "imgui_wrap.h"
#include "tc0480scp.h"
#include "sim_core.h"
#include "sim_hierarchy.h"

#include "gfx_cache.h"
#include "F2.h"
#include "F2___024root.h"

class TC0480SCPFlagsWindow : public Window
{
public:
    TC0480SCPFlagsWindow() : Window("TC0480SCP Flags") {}

    void Init() {}

    void Draw()
    {
        int16_t ctrl[32];
        for (int i = 0; i < 32; i++)
        {
            ctrl[i] = G_F2_SIGNAL(tc0480scp, ctrl)[i];
        }

        ImGui::LabelText("BG0", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[0], ctrl[4],
                         ctrl[8], ctrl[16], ctrl[20]);
        ImGui::LabelText("BG1", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[1], ctrl[5],
                         ctrl[9], ctrl[17], ctrl[21]);
        ImGui::LabelText("BG2", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[2], ctrl[6],
                         ctrl[10], ctrl[18], ctrl[22]);
        ImGui::LabelText("BG3", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[3], ctrl[7],
                         ctrl[11], ctrl[19], ctrl[23]);
        ImGui::LabelText("FG0", "X:%d Y:%d", ctrl[12], ctrl[13]);
        ImGui::LabelText("Status", "%04X %04X", ctrl[14], ctrl[15]);
    }
};

TC0480SCPFlagsWindow s_TC0480SCPFlagsWindow;


class TC0480SCPViewWindow : public Window
{
public:
    int m_layer = 0;

    TC0480SCPViewWindow() : Window("TC0480SCP View") {}

    void Init() {}
    
    bool IsWide()
    {
        const uint16_t ctrl = G_F2_SIGNAL(tc0480scp, ctrl)[15];
        return (ctrl & (1 << 7)) != 0;
    }
    
    uint16_t MemWord(uint32_t addr)
    {
        addr = (addr & 0xffff) >> 1;
        uint8_t high = G_F2_SIGNAL(scn_mux_ram, ram_h)[addr];
        uint8_t low = G_F2_SIGNAL(scn_mux_ram, ram_l)[addr];

        return (high << 8) | low;
    }

    uint32_t LayerBaseAddr()
    {
        return m_layer * (IsWide() ? 0x2000 : 0x1000);
    }

    void Draw()
    {
        const bool wide = IsWide();
        const int num_columns = wide ? 64 : 32;
        const int num_rows = 32;

        const char *layer_names[4] = { "BG0", "BG1", "BG2", "BG3" };
        ImGui::Combo("Layer", &m_layer, layer_names, 4);

        if (ImGui::BeginTable("layer", num_columns))
        {
            uint32_t base_addr = LayerBaseAddr();
            for( int y = 0; y < num_rows; y++ )
            {
                ImGui::TableNextRow();
                for( int x = 0; x < num_columns; x++ )
                {
                    uint32_t addr = base_addr + (((y * num_columns) + x) * 4);
                    uint16_t attrib = MemWord(addr);
                    uint16_t code = MemWord(addr + 2) & 0x7fff;
                    ImGui::TableNextColumn();
                    SDL_Texture *tex = g_sim_core.gfx_cache->GetTexture(MemoryRegion::SCN1_ROM, GfxCacheFormat::TC0480SCP, code, attrib & 0xff);
                    ImGui::Image((ImTextureID)tex, ImVec2(32, 32));
                    ImGui::Text("%04X", code);
                }
            }
            ImGui::EndTable();
        }
    }
};

TC0480SCPViewWindow s_TC0480SCPViewWindow;
