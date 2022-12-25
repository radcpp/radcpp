#include "HelloWorld.h"
#include "rad/Core/Logging.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);
    float windowScale = app.GetDisplayDPI(0) / 96.0f;

    HelloWorld mainWindow;
    mainWindow.Create("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        int(800 * windowScale), int(600 * windowScale),
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    mainWindow.Init();

    for (const std::string& path : StrSplit(app.GetEnv("PATH"), ";", true))
    {
        LogPrint("Global", LogLevel::Info, "PATH=%s", path.c_str());
    }

    return app.Run();
}
