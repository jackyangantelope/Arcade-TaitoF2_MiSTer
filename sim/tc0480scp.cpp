#include <SDL.h>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_wrap.h"
#include "tc0480scp.h"

#include "F2.h"
#include "F2___024root.h"

extern F2* top;
        

void draw_480scp_window()
{
    if( !ImGui::Begin("TC0480SCP") )
    {
        ImGui::End();
        return;
    }

    int16_t ctrl[32];
    for( int i = 0; i < 32; i++ )
    {
        ctrl[i] = top->rootp->F2__DOT__tc0480scp__DOT__ctrl[i];
    }

    ImGui::LabelText("BG0", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[0], ctrl[4], ctrl[8], ctrl[16], ctrl[20]);
    ImGui::LabelText("BG1", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[1], ctrl[5], ctrl[9], ctrl[17], ctrl[21]);
    ImGui::LabelText("BG2", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[2], ctrl[6], ctrl[10], ctrl[18], ctrl[22]);
    ImGui::LabelText("BG3", "X:%d Y:%d Z:%d DX:%d DY:%d", ctrl[3], ctrl[7], ctrl[11], ctrl[19], ctrl[23]);
    ImGui::LabelText("FG0", "X:%d Y:%d", ctrl[12], ctrl[13]);
    ImGui::LabelText("Status", "%04X %04X", ctrl[14], ctrl[15]);

    ImGui::End();
}

