#include <SDL.h>

#include "imgui_wrap.h"
#include "sim_hierarchy.h"
#include "sim_core.h"

#include "F2.h"
#include "F2___024root.h"

class TC0360PRIWindow : public Window
{
public:
    TC0360PRIWindow() : Window("TC0360PRI") {}

    void Init() {};
    void Draw()
    {
        uint16_t color_in0 = G_F2_SIGNAL(tc0360pri, color_in0);
        uint16_t color_in1 = G_F2_SIGNAL(tc0360pri, color_in1);
        uint16_t color_in2 = G_F2_SIGNAL(tc0360pri, color_in2);
        uint8_t ctrl[16];
        for (int i = 0; i < 16; i++)
        {
            ctrl[i] = G_F2_SIGNAL(tc0360pri, ctrl)[i];
        }

        ImGui::Text("Color0: %03X  Sel0: %d", color_in0 & 0xfff,
                    (color_in0 >> 12) & 0x3);
        ImGui::Text("Color1: %03X  Sel1: %d", color_in1 & 0xfff,
                    (color_in1 >> 12) & 0x3);
        ImGui::Text("Color2: %03X  Sel2: %d", color_in2 & 0xfff,
                    (color_in2 >> 12) & 0x3);

        ImGui::Text("Prio0: %X %X %X %X", (ctrl[4] >> 0) & 0xf,
                    (ctrl[4] >> 4) & 0xf, (ctrl[5] >> 0) & 0xf,
                    (ctrl[5] >> 4) & 0xf);

        ImGui::Text("Prio1: %X %X %X %X", (ctrl[6] >> 0) & 0xf,
                    (ctrl[6] >> 4) & 0xf, (ctrl[7] >> 0) & 0xf,
                    (ctrl[7] >> 4) & 0xf);

        ImGui::Text("Prio2: %X %X %X %X", (ctrl[8] >> 0) & 0xf,
                    (ctrl[8] >> 4) & 0xf, (ctrl[9] >> 0) & 0xf,
                    (ctrl[9] >> 4) & 0xf);

        for (int i = 0; i < 8; i++)
        {
            ImGui::Text("%04X %04X", ctrl[i * 2], ctrl[(i * 2) + 1]);
        }
    }
};

TC0360PRIWindow s_TC0360PRIWindow;

