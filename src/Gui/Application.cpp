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
    for (int i = 0; i < argc; ++i)
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

void Application::RegisterEventHandler(EventHandler* handler)
{
    m_eventHandlers.push_back(handler);
}

void Application::UnregisterEventHandler(EventHandler* handler)
{
    m_eventHandlers.remove(handler);
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

        for (EventHandler* handler : m_eventHandlers)
        {
            handler->OnIdle();
        }

        if (m_eventHandlers.empty())
        {
            m_exitEventLoop = true;
        }
    }
    return m_exitCode;
}

bool Application::OnEvent(const SDL_Event& event)
{
    for (EventHandler* handler : m_eventHandlers)
    {
        if (handler->OnEvent(event))
        {
            return true;
        }
    }

    switch (event.type)
    {
    case SDL_QUIT:
        m_exitEventLoop = true;
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

float Application::GetDisplayDPI(int displayIndex)
{
    float ddpi = 96.0f;
    float hdpi = 96.0f;
    float vdpi = 96.0f;
    if (SDL_GetDisplayDPI(displayIndex, &ddpi, &hdpi, &vdpi) != 0)
    {
        LogPrint("Global", LogLevel::Error, "SDL_GetDisplayDPI: %s", SDL_GetError());
    }
    return (hdpi + vdpi) / 2.0f;
}
