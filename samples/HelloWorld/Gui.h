#pragma once

#include "rad/Gui/Application.h"
#include "rad/Gui/Window.h"

#include "rad/Core/Common.h"
#include "rad/Core/Memory.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_sdlrenderer.h"

class Gui : public RefCounted<Gui>
{
public:
    Gui(Window* window, SDL_Renderer* renderer);
    ~Gui();

    void BeginFrame();
    void EndFrame();
    void Render();

private:
    Window* m_window;
    SDL_Renderer* m_renderer;

}; // class Gui
