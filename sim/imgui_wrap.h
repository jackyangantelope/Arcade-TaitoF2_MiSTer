#if !defined(IMGUI_WRAP_H)
#define IMGUI_WRAP_H 1

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_internal.h"

#include <string>

bool imgui_init(const char *title);
bool imgui_begin_frame();
void imgui_end_frame();
void imgui_set_title(const char *title);

uint32_t imgui_get_buttons();

struct SDL_Renderer;
SDL_Renderer *imgui_get_renderer();


class Window
{
public:
    Window(const char *name);
    virtual ~Window();

    void Update();

    virtual void Draw() = 0;

    std::string m_title;
    bool m_enabled;

    Window *m_next;
    static Window *s_head;
};

#endif // IMGUI_WRAP_H
