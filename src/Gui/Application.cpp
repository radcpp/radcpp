#include "rad/Gui/Application.h"
#include "rad/Gui/Window.h"

#include "rad/Core/Exception.h"
#include "rad/Core/Logging.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

Application* g_app = nullptr;

Application::Application(int argc, char** argv)
{
    assert(g_app == nullptr);
    g_app = this;

#if defined(_WIN32)
    ::SetConsoleOutputCP(65001);
#endif

    std::string commandLine;
    for (int i = 0; i < argc; i++)
    {
        m_commandLineArgs.push_back(argv[i]);
        commandLine += std::string(argv[i]) + " ";
    }
    commandLine.pop_back();
    LogPrint("Global", LogLevel::Info, "Command line: %s", commandLine.c_str());

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        LogPrint("Global", LogLevel::Error, "SDL_Init failed: %s", SDL_GetError());
        throw std::runtime_error(SDL_GetError());
    }

    SDL_version linkedVersion = {};
    SDL_GetVersion(&linkedVersion);
    LogPrint("Global", LogLevel::Info, "SDL initialized: Version: %d.%d.%d",
        linkedVersion.major, linkedVersion.minor, linkedVersion.patch);

    LogPrint("Global", LogLevel::Info, "Platform: %s", SDL_GetPlatform());
    char* basePath = SDL_GetBasePath();
    LogPrint("Global", LogLevel::Info, "BasePath: %s", basePath);
    SDL_free(basePath);

    UpdateDisplays();
}

Application::~Application()
{
    SDL_Quit();
    LogPrint("Global", LogLevel::Info, "SDL Quited.");
}

Application* Application::GetInstance()
{
    return g_app;
}

void Application::Register(Window* window)
{
    m_windows[window->GetID()] = window;
}

void Application::UnRegister(Window* window)
{
    m_windows.erase(window->GetID());
}

int Application::Run()
{
    while (!m_exitEventLoop)
    {
        SDL_Event event = {};
        while (SDL_PollEvent(&event))
        {
            OnEvent(event);
        }

        for (auto& idHandlePair : m_windows)
        {
            if (idHandlePair.second)
            {
                idHandlePair.second->OnRender();
            }
        }

        if (m_windows.empty())
        {
            m_exitEventLoop = true;
        }
    }
    return m_exitCode;
}

Window* Application::GetWindowFromId(Uint32 id)
{
    auto iter = m_windows.find(id);
    if (iter != m_windows.end())
    {
        return iter->second;
    }
    return nullptr;
}

bool Application::OnEvent(const SDL_Event& event)
{
    for (auto& idHandlePair : m_windows)
    {
        if (idHandlePair.second)
        {
            if (idHandlePair.second->OnEvent(event))
            {
                return true;
            }
        }
    }

    switch (event.type)
    {
    case SDL_QUIT:
        m_exitEventLoop = true;
        break;
    case SDL_WINDOWEVENT:
        if (Window* window = GetWindowFromId(event.window.windowID))
        {
            window->OnWindowEvent(event.window);
        }
        break;
    case SDL_KEYDOWN:
        if (Window* window = GetWindowFromId(event.key.windowID))
        {
            window->OnKeyDown(event.key);
        }
        break;
    case SDL_KEYUP:
        if (Window* window = GetWindowFromId(event.key.windowID))
        {
            window->OnKeyUp(event.key);
        }
        break;
    case SDL_TEXTEDITING:
        if (Window* window = GetWindowFromId(event.edit.windowID))
        {
            window->OnTextEditing(event.edit);
        }
        break;
    case SDL_TEXTINPUT:
        if (Window* window = GetWindowFromId(event.text.windowID))
        {
            window->OnTextInput(event.text);
        }
        break;
    case SDL_MOUSEMOTION:
        if (Window* window = GetWindowFromId(event.motion.windowID))
        {
            window->OnMouseMove(event.motion);
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (Window* window = GetWindowFromId(event.button.windowID))
        {
            window->OnMouseButtonDown(event.button);
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (Window* window = GetWindowFromId(event.button.windowID))
        {
            window->OnMouseButtonUp(event.button);
        }
        break;
    case SDL_MOUSEWHEEL:
        if (Window* window = GetWindowFromId(event.wheel.windowID))
        {
            window->OnMouseWheel(event.wheel);
        }
        break;

    case SDL_APP_TERMINATING:
        LogPrint("Global", LogLevel::Info, "SDL_APP_TERMINATING: The application is being terminated by the OS.");
        break;
    case SDL_APP_LOWMEMORY:
        LogPrint("Global", LogLevel::Info, "SDL_APP_LOWMEMORY: The application is low on memory, free memory if possible.");
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_WILLENTERBACKGROUND: The application is about to enter the background.");
        break;
    case SDL_APP_DIDENTERBACKGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_DIDENTERBACKGROUND: The application did enter the background and may not get CPU for some time.");
        break;
    case SDL_APP_WILLENTERFOREGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_WILLENTERFOREGROUND: The application is about to enter the foreground.");
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_DIDENTERFOREGROUND: The application is now interactive.");
        break;
    case SDL_LOCALECHANGED:
        LogPrint("Global", LogLevel::Info, "SDL_LOCALECHANGED: The user's locale preferences have changed.");
        break;
    case SDL_DISPLAYEVENT:
        LogPrint("Global", LogLevel::Info, "SDL_DISPLAYEVENT: Display state change.");
        UpdateDisplays();
        break;
    }

    return true;
}

void Application::UpdateDisplays()
{
    m_displayInfos.resize(SDL_GetNumVideoDisplays());
    for (int displayIndex = 0; displayIndex < static_cast<int>(m_displayInfos.size()); displayIndex++)
    {
        DisplayInfo& displayInfo = m_displayInfos[displayIndex];
        displayInfo.name = SDL_GetDisplayName(displayIndex);
        SDL_GetDisplayBounds(displayIndex, &displayInfo.bounds);
        SDL_GetDisplayUsableBounds(displayIndex, &displayInfo.usableBounds);
        SDL_GetDisplayDPI(displayIndex, &displayInfo.ddpi, &displayInfo.hdpi, &displayInfo.vdpi);
        displayInfo.modes.resize(SDL_GetNumDisplayModes(displayIndex));
        for (int modeIndex = 0; modeIndex < displayInfo.modes.size(); modeIndex++)
        {
            SDL_GetDisplayMode(displayIndex, modeIndex, &displayInfo.modes[modeIndex]);
        }
    }
}

std::string Application::GetEnv(std::string_view name)
{
    std::string value;
#ifdef _WIN32
    std::wstring nameWide = StrU8ToWide(name);
    wchar_t* pBuffer = nullptr;
    size_t bufferSize = 0;
    errno_t err = _wdupenv_s(&pBuffer, &bufferSize, nameWide.data());
    if (err == 0)
    {
        if (pBuffer)
        {
            value = StrWideToU8(pBuffer);
            free(pBuffer);
        }
    }
#else
    var = std::getenv(varName.data());
#endif
    return value;
}
