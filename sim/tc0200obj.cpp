#include <SDL.h>

#include "imgui_wrap.h"
#include "tc0200obj.h"
#include "sim_core.h"
#include "sim_hierarchy.h"

#include "gfx_cache.h"

#include "F2.h"
#include "F2___024root.h"

void get_obj_inst(uint16_t index, TC0200OBJ_Inst *inst)
{
    uint8_t *inst_data = (uint8_t *)inst;

    uint16_t offset = index * 8;

    for (int i = 0; i < 8; i++)
    {
        inst_data[(i * 2) + 0] = G_F2_SIGNAL(obj_ram, ram_l).m_storage[offset + i];
        inst_data[(i * 2) + 1] = G_F2_SIGNAL(obj_ram, ram_h).m_storage[offset + i];
    }
}

uint16_t extended_code(uint16_t index, uint16_t code)
{
    if (G_F2_SIGNAL(cfg_190fmc))
    {
        uint8_t *ctrl = G_F2_SIGNAL(tc0190fmc, ctrl).m_storage;
        uint8_t sel = (code >> 10) & 7;
        switch (sel)
        {
        case 0:
        case 1:
            return (ctrl[2] << 11) | (code & 0x7ff);

        case 2:
        case 3:
            return (ctrl[3] << 11) | (code & 0x7ff);

        default:
            return (ctrl[sel] << 10) | (code & 0x3ff);
        }
    }

    if (G_F2_SIGNAL(cfg_obj_extender) == 1)
    {
        uint8_t ext = G_F2_SIGNAL(tc0200obj_extender, extension_ram, ram).m_storage[index];
        return (code & 0xff) | (ext << 8);
    }
    else
    {
        return code;
    }
}

static void bullet(int x)
{
    if (x != 0)
        ImGui::Bullet();
}

class TC0200OBJ_Window : public Window
{
  public:
    int m_bank = 0;
    bool m_hide_empty = false;

    TC0200OBJ_Window() : Window("TC0200OBJ Instances")
    {
    }

    void Init() {};

    void Draw()
    {
        const char *bank_names[4] = {"0x0000", "0x4000", "0x8000", "0xC000"};

        ImGui::Combo("Bank", &m_bank, bank_names, 4);

        ImGui::Checkbox("Hide Empty", &m_hide_empty);

        if (ImGui::BeginTable("obj", 26,
                              ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg))
        {
            uint32_t colflags = ImGuiTableColumnFlags_AngledHeader;
            ImGui::TableSetupColumn("", colflags & ~ImGuiTableColumnFlags_AngledHeader);
            ImGui::TableSetupColumn("Code", colflags);
            ImGui::TableSetupColumn("Code (Ext)", colflags);
            ImGui::TableSetupColumn("X", colflags);
            ImGui::TableSetupColumn("Y", colflags);
            ImGui::TableSetupColumn("Cmd", colflags);
            ImGui::TableSetupColumn("Latch Extra", colflags);
            ImGui::TableSetupColumn("Latch Master", colflags);
            ImGui::TableSetupColumn("Ignore Extra", colflags);
            ImGui::TableSetupColumn("Ignore All", colflags);
            ImGui::TableSetupColumn("Flip X", colflags);
            ImGui::TableSetupColumn("Flip Y", colflags);
            ImGui::TableSetupColumn("Is Seq", colflags);
            ImGui::TableSetupColumn("Latch Y", colflags);
            ImGui::TableSetupColumn("Inc Y", colflags);
            ImGui::TableSetupColumn("Latch X", colflags);
            ImGui::TableSetupColumn("Inc X", colflags);
            ImGui::TableSetupColumn("Latch Color", colflags);
            ImGui::TableSetupColumn("Color", colflags);
            ImGui::TableSetupColumn("Unk1", colflags);
            ImGui::TableSetupColumn("Unk2", colflags);
            ImGui::TableSetupColumn("Unk3", colflags);
            ImGui::TableSetupColumn("Cmd", colflags);
            ImGui::TableSetupColumn("Zoom X", colflags);
            ImGui::TableSetupColumn("Zoom Y", colflags);
            ImGui::TableSetupColumn("Debug", colflags);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableAngledHeadersRow();

            TC0200OBJ_Inst insts[1024];
            uint8_t latched_color[1024];
            uint16_t extcode[1024];

            const int bank_base = m_bank * 1024;

            uint8_t last_color = 0;
            std::vector<int> valid_insts;
            valid_insts.reserve(1024);

            for (int i = 0; i < 1024; i++)
            {
                get_obj_inst(bank_base + i, &insts[i]);
                extcode[i] = extended_code(bank_base + i, insts[i].code);
                if (insts[i].latch_color)
                {
                    latched_color[i] = last_color;
                }
                else
                {
                    latched_color[i] = last_color = insts[i].color;
                }

                if (m_hide_empty)
                {
                    if (insts[i].code || insts[i].has_cmd || insts[i].latch_extra || insts[i].latch_master)
                    {
                        valid_insts.push_back(i);
                    }
                }
                else
                {
                    valid_insts.push_back(i);
                }
            }

            int hovered = ImGui::TableGetHoveredRow();

            ImGuiListClipper clipper;
            clipper.Begin(valid_insts.size());
            int row_count = 1;
            int tooltip_idx = -1;
            while (clipper.Step())
            {
                for (uint16_t i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    int index = valid_insts[i];

                    if (row_count == hovered)
                    {
                        tooltip_idx = index;
                    }
                    row_count++;

                    TC0200OBJ_Inst &inst = insts[index];
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%4u", index);
                    ImGui::TableNextColumn();
                    ImGui::Text("%04X", inst.code);
                    ImGui::TableNextColumn();
                    ImGui::Text("%04X", extcode[index]);
                    ImGui::TableNextColumn();
                    ImGui::Text("%4d", inst.x);
                    ImGui::TableNextColumn();
                    ImGui::Text("%4d", inst.y);
                    ImGui::TableNextColumn();
                    bullet(inst.has_cmd);
                    ImGui::TableNextColumn();
                    bullet(inst.latch_extra);
                    ImGui::TableNextColumn();
                    bullet(inst.latch_master);
                    ImGui::TableNextColumn();
                    bullet(inst.ignore_extra);
                    ImGui::TableNextColumn();
                    bullet(inst.ignore_all);
                    ImGui::TableNextColumn();
                    bullet(inst.flipx);
                    ImGui::TableNextColumn();
                    bullet(inst.flipy);
                    ImGui::TableNextColumn();
                    bullet(inst.is_seq);
                    ImGui::TableNextColumn();
                    bullet(inst.latch_y);
                    ImGui::TableNextColumn();
                    bullet(inst.inc_y);
                    ImGui::TableNextColumn();
                    bullet(inst.latch_x);
                    ImGui::TableNextColumn();
                    bullet(inst.inc_x);
                    ImGui::TableNextColumn();
                    bullet(inst.latch_color);
                    ImGui::TableNextColumn();
                    ImGui::Text("%02X", inst.color);
                    ImGui::TableNextColumn();
                    bullet(inst.unk1);
                    ImGui::TableNextColumn();
                    bullet(inst.unk2);
                    ImGui::TableNextColumn();
                    bullet(inst.unk3);
                    ImGui::TableNextColumn();
                    ImGui::Text("%04X", inst.cmd_bits);
                    ImGui::TableNextColumn();
                    ImGui::Text("%02X", inst.zoom_x);
                    ImGui::TableNextColumn();
                    ImGui::Text("%02X", inst.zoom_y);
                    ImGui::TableNextColumn();
                    bool is_debug = index == g_sim_core.top->obj_debug_idx;
                    char id[16];
                    snprintf(id, 16, "##debug%d", index);
                    if (ImGui::RadioButton(id, is_debug))
                    {
                        if (is_debug)
                            g_sim_core.top->obj_debug_idx = -1;
                        else
                            g_sim_core.top->obj_debug_idx = index;
                    }
                }
            }

            if (tooltip_idx >= 0)
            {
                uint16_t code = extcode[tooltip_idx];
                if (code != 0)
                {
                    SDL_Texture *tex = g_sim_core.gfx_cache->GetTexture(MemoryRegion::OBJ_ROM, GfxCacheFormat::TC0200OBJ, code,
                                                                        latched_color[tooltip_idx]);
                    ImGui::BeginTooltip();
                    ImGui::LabelText("Code", "%04X", code);
                    ImGui::LabelText("Color", "%02X", latched_color[tooltip_idx]);
                    ImGui::Image((ImTextureID)tex, ImVec2(64, 64));
                    ImGui::End();
                }
            }

            ImGui::EndTable();
        }
    }
};

TC0200OBJ_Window s_TC0200OBJ_Window;

class TC0200OBJ_Preview_Window : public Window
{
  public:
    TC0200OBJ_Preview_Window() : Window("TC0200OBJ Preview")
    {
    }
    int m_color = 0;

    void Init() {};
    void Draw()
    {
        ImGui::SliderInt("Color", &m_color, 0, 0xff, "%02X");
        m_color &= 0xff;

        if (ImGui::BeginTable("obj_list", 17,
                              ImGuiTableFlags_HighlightHoveredColumn | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY |
                                  ImGuiTableFlags_SizingFixedFit))
        {
            ImGuiTableColumnFlags colflags = 0;
            ImGui::TableSetupColumn("", colflags);
            ImGui::TableSetupColumn("0", colflags);
            ImGui::TableSetupColumn("1", colflags);
            ImGui::TableSetupColumn("2", colflags);
            ImGui::TableSetupColumn("3", colflags);
            ImGui::TableSetupColumn("4", colflags);
            ImGui::TableSetupColumn("5", colflags);
            ImGui::TableSetupColumn("6", colflags);
            ImGui::TableSetupColumn("7", colflags);
            ImGui::TableSetupColumn("8", colflags);
            ImGui::TableSetupColumn("9", colflags);
            ImGui::TableSetupColumn("A", colflags);
            ImGui::TableSetupColumn("B", colflags);
            ImGui::TableSetupColumn("C", colflags);
            ImGui::TableSetupColumn("D", colflags);
            ImGui::TableSetupColumn("E", colflags);
            ImGui::TableSetupColumn("F", colflags);
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(0x10000 / 16);
            while (clipper.Step())
            {
                for (uint16_t index = clipper.DisplayStart; index < clipper.DisplayEnd; index++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%03Xx", index);
                    uint16_t base_code = index * 16;
                    for (int i = 0; i < 16; i++)
                    {
                        ImGui::TableNextColumn();
                        SDL_Texture *tex =
                            g_sim_core.gfx_cache->GetTexture(MemoryRegion::OBJ_ROM, GfxCacheFormat::TC0200OBJ, base_code + i, m_color);
                        ImGui::Image((ImTextureID)tex, ImVec2(32, 32));
                    }
                }
            }

            ImGui::EndTable();
        }
    }
};

TC0200OBJ_Preview_Window s_TC0200OBJ_Preview_Window;
