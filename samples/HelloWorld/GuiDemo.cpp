#include "GuiDemo.h"
#include "rad/Core/Logging.h"

GuiDemo::GuiDemo()
{
    float windowScale = GetDisplayDPI() / 96.0f;
    Create("Gui Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        int(1600 * windowScale), int(900 * windowScale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_ALLOW_HIGHDPI);

    m_renderer = SDL_CreateRenderer(m_handle, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer)
    {
        SDL_RendererInfo renderInfo = {};
        SDL_GetRendererInfo(m_renderer, &renderInfo);
        LogPrint("Gui", LogLevel::Info, "SDL_CreateRenderer: %s", renderInfo.name);
        m_gui = new Gui(this, m_renderer);
    }
    else
    {
        LogPrint("Gui", LogLevel::Error, "SDL_CreateRenderer: %s", SDL_GetError());
    }
}

GuiDemo::~GuiDemo()
{
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
}

bool GuiDemo::OnEvent(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
    return false;
}

void GuiDemo::OnKeyDown(const SDL_KeyboardEvent& keyDown)
{
    if (keyDown.keysym.sym == SDLK_F1)
    {
        m_showDemoWindow = !m_showDemoWindow;
    }
}

void GuiDemo::OnKeyUp(const SDL_KeyboardEvent& keyUp)
{
}

void GuiDemo::OnRender()
{
    m_gui->BeginFrame();
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
    m_gui->EndFrame();

    SDL_SetRenderDrawColor(m_renderer,
        Uint8(0), Uint8(0), Uint8(0), Uint8(0));
    SDL_RenderClear(m_renderer);
    m_gui->Render();
    SDL_RenderPresent(m_renderer);
}
