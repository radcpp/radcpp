#include "rad/Gui/Application.h"
#include "rad/Gui/Window.h"

#include "WindowTest.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    float windowScale = app.GetDisplayDPI(0) / 96.0f;

    WindowTest windowTest("WindowTest");
    windowTest.Create("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        int(800 * windowScale), int(600 * windowScale), 0);

    for (const std::string& path : StrSplit(app.GetEnv("PATH"), ";", true))
    {
        LogPrint("Global", LogLevel::Info, "PATH=%s", path.c_str());
    }

    return app.Run();
}
