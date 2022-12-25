#include "HelloWorld.h"
#include "rad/Core/Logging.h"

#ifdef _WIN32
#include <sdkddkver.h>
#endif
#include "boost/process.hpp"

HelloWorld::HelloWorld()
{
}

HelloWorld::~HelloWorld()
{
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
}

bool HelloWorld::Init()
{
    m_renderer = SDL_CreateRenderer(m_handle, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer)
    {
        SDL_RendererInfo renderInfo = {};
        SDL_GetRendererInfo(m_renderer, &renderInfo);
        LogPrint("Gui", LogLevel::Info, "SDL_CreateRenderer: %s", renderInfo.name);
        m_gui = new Gui(this, m_renderer);
        return true;
    }
    else
    {
        LogPrint("Gui", LogLevel::Error, "SDL_CreateRenderer: %s", SDL_GetError());
        return false;
    }
}

bool HelloWorld::OnEvent(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
    return false;
}

void HelloWorld::OnKeyDown(const SDL_KeyboardEvent& keyDown)
{
    if (keyDown.keysym.sym == SDLK_F1)
    {
        m_showDemoWindow = !m_showDemoWindow;
    }
    else if (keyDown.keysym.sym == SDLK_F2)
    {
        m_showEnvironment = !m_showEnvironment;
    }
}

void HelloWorld::OnKeyUp(const SDL_KeyboardEvent& keyUp)
{
}

class EnvWindow : public RefCounted<EnvWindow>
{
public:
    EnvWindow()
    {
    }

    ~EnvWindow()
    {
    }

    void Render(bool* open)
    {
        if (m_env.empty())
        {
            m_env = boost::this_process::environment();
        }
        ImGui::Begin("Environment.Window", open);
        if (ImGui::BeginTable("Environment.Table", 2,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_Resizable |
            ImGuiTableFlags_SizingFixedFit))
        {
            for (const auto& env : m_env)
            {
                if (!env.empty() && !env.get_name().empty())
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text(env.get_name().c_str());
                    if (StrCaseEqual(env.get_name(), "Path"))
                    {
                        if (m_pathFolders.empty())
                        {
                            m_pathFolders = StrSplit(env.to_string(), ";");
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::BeginTable("Environment.Table.PathFolders", 1,
                            ImGuiTableFlags_BordersInnerH))
                        {
                            for (const std::string& folder : m_pathFolders)
                            {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text(folder.c_str());
                            }
                            ImGui::EndTable();
                        }
                    }
                    else
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text(env.to_string().c_str());
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }

    boost::process::native_environment m_env;
    std::vector<std::string> m_pathFolders;

}; // class EnvWindow

void HelloWorld::OnRender()
{
    m_gui->BeginFrame();
    if (m_showDemoWindow)
    {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
    }
    if (m_showEnvironment)
    {
        if (!m_envWindow)
        {
            m_envWindow = new EnvWindow();
        }
        m_envWindow->Render(&m_showEnvironment);
        if (!m_showEnvironment)
        {
            m_envWindow.reset();
        }
    }
    m_gui->EndFrame();

    SDL_SetRenderDrawColor(m_renderer,
        Uint8(0), Uint8(0), Uint8(0), Uint8(0));
    SDL_RenderClear(m_renderer);
    m_gui->Render();
    SDL_RenderPresent(m_renderer);
}
