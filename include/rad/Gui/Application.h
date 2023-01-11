#pragma once

#include "rad/Core/Common.h"
#include "rad/Core/Containers.h"

#include "SDL2/SDL.h"

struct DisplayInfo
{
    std::string name;
    SDL_Rect bounds;
    SDL_Rect usableBounds;
    float ddpi;
    float hdpi;
    float vdpi;
    std::vector<SDL_DisplayMode> modes;
};

class EventHandler
{
public:
    virtual bool OnEvent(const SDL_Event& event) = 0;
    virtual void OnIdle() {}
}; // class EventHandler

class Window;

class Application
{
public:
    Application(int argc, char** argv);
    ~Application();

    static Application* GetInstance();

    void RegisterEventHandler(EventHandler* handler);
    void UnregisterEventHandler(EventHandler* handler);

    int Run();
    bool OnEvent(const SDL_Event& event);

    void SetExitEventLoop(bool exit = true) { m_exitEventLoop = exit; }
    void SetExitCode(int exitCode) { m_exitCode = exitCode; }

    const std::vector<std::string>& GetCommandLineArgs() const { return m_commandLineArgs; }
    const std::vector<DisplayInfo>& GetDisplays() const { return m_displayInfos; }

    static std::string GetEnv(std::string_view name);

    // Return the average of hdpi and vdpi.
    float GetDisplayDPI(int displayIndex);

private:
    void UpdateDisplays();

    std::vector<std::string> m_commandLineArgs;
    std::vector<DisplayInfo> m_displayInfos;

    bool m_exitEventLoop = false;
    int m_exitCode = 0;

    // Windows that are registered for event loop
    std::list<EventHandler*> m_eventHandlers;

}; // class Application
