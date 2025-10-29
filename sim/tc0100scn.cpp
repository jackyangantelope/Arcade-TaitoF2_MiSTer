#include <SDL.h>

#include "imgui_wrap.h"
#include "tc0480scp.h"
#include "sim_core.h"
#include "sim_hierarchy.h"

#include "gfx_cache.h"
#include "F2.h"
#include "F2___024root.h"

class TC0100SCNViewWindow : public Window
{
  public:
    int m_layer = 0;

    TC0100SCNViewWindow() : Window("TC0100SCN View", ImGuiWindowFlags_AlwaysAutoResize)
    {
    }

    void Init()
    {
    }

    bool IsWide()
    {
        const uint16_t ctrl = G_F2_SIGNAL(scn0, ctrl)[6];
        return (ctrl & (1 << 4)) != 0;
    }

    uint16_t MemWord(uint32_t addr)
    {
        addr = (addr & 0xffff) >> 1;
        uint8_t high = G_F2_SIGNAL(scn_ram_0, ram_h)[addr];
        uint8_t low = G_F2_SIGNAL(scn_ram_0, ram_l)[addr];

        return (high << 8) | low;
    }

    uint32_t LayerBaseAddr()
    {
        return m_layer * 0x8000;
    }

    void Draw()
    {
        const bool wide = IsWide();
        const int num_columns = wide ? 128 : 64;
        const int num_rows = 64;

        const char *layer_names[3] = {"BG0", "BG1", "FG0"};
        ImGui::Combo("Layer", &m_layer, layer_names, 3);

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(1.0f, 1.0f));
        if (ImGui::BeginTable("layer", num_columns, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedSame,
                              ImVec2(760, 560)))
        {
            if (m_layer == 2)
            {
                MemorySlice gfx_normal(g_sim_core.Memory(MemoryRegion::SCN_0), 0x6000, 0x2000);
                MemorySlice gfx_wide(g_sim_core.Memory(MemoryRegion::SCN_0), 0x12000, 0x2000);

                uint32_t base_addr = 0x4000;
                for (int y = 0; y < num_rows; y++)
                {
                    ImGui::TableNextRow();
                    for (int x = 0; x < num_columns; x++)
                    {
                        uint32_t addr = base_addr + (((y * num_columns) + x) * 2);
                        uint16_t attrib = MemWord(addr);
                        ImGui::TableNextColumn();
                        SDL_Texture *tex = g_sim_core.gfx_cache->GetTexture(wide ? gfx_wide : gfx_normal, GfxCacheFormat::TC0100SCN_FG,
                                                                            attrib & 0xff, (attrib >> 8) & 0x3f);
                        ImGui::Image((ImTextureID)tex, ImVec2(16, 16));
                    }
                }

                int hover_y = ImGui::TableGetHoveredRow();
                int hover_x = ImGui::TableGetHoveredColumn();

                if (hover_y >= 0 && hover_x >= 0)
                {
                    uint32_t addr = base_addr + (((hover_y * num_columns) + hover_x) * 2);
                    uint16_t attrib = MemWord(addr);

                    ImGui::BeginTooltip();
                    ImGui::LabelText("Code", "%02X", attrib & 0xff);
                    ImGui::LabelText("Attrib", "%02X", attrib >> 8);
                    ImGui::LabelText("Address", "%04X", addr);
                    SDL_Texture *tex = g_sim_core.gfx_cache->GetTexture(wide ? gfx_wide : gfx_normal, GfxCacheFormat::TC0100SCN_FG,
                                                                        attrib & 0xff, (attrib >> 8) & 0x3f);
                    ImGui::Image((ImTextureID)tex, ImVec2(64, 64));
                    ImGui::End();
                }
            }
            else
            {
                uint32_t base_addr = LayerBaseAddr();
                for (int y = 0; y < num_rows; y++)
                {
                    ImGui::TableNextRow();
                    for (int x = 0; x < num_columns; x++)
                    {
                        uint32_t addr = base_addr + (((y * num_columns) + x) * 4);
                        uint16_t attrib = MemWord(addr);
                        uint16_t code = MemWord(addr + 2) & 0x7fff;
                        ImGui::TableNextColumn();
                        SDL_Texture *tex =
                            g_sim_core.gfx_cache->GetTexture(MemoryRegion::SCN0_ROM, GfxCacheFormat::TC0100SCN, code, attrib & 0xff);
                        ImGui::Image((ImTextureID)tex, ImVec2(16, 16));
                    }
                }

                int hover_y = ImGui::TableGetHoveredRow();
                int hover_x = ImGui::TableGetHoveredColumn();

                if (hover_y >= 0 && hover_x >= 0)
                {
                    uint32_t addr = base_addr + (((hover_y * num_columns) + hover_x) * 4);
                    uint16_t attrib = MemWord(addr);
                    uint16_t code = MemWord(addr + 2) & 0x7fff;

                    ImGui::BeginTooltip();
                    ImGui::LabelText("Code", "%04X", code);
                    ImGui::LabelText("Attrib", "%04X", attrib);
                    ImGui::LabelText("Address", "%04X", addr);
                    SDL_Texture *tex =
                        g_sim_core.gfx_cache->GetTexture(MemoryRegion::SCN0_ROM, GfxCacheFormat::TC0100SCN, code, attrib & 0xff);
                    ImGui::Image((ImTextureID)tex, ImVec2(64, 64));
                    ImGui::End();
                }
            }
            ImGui::EndTable();
        }
        ImGui::PopStyleVar();
    }
};

TC0100SCNViewWindow s_TC0100SCNViewWindow;
