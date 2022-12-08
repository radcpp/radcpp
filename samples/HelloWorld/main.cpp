#include "rad/Gui/Application.h"
#include "rad/Gui/Window.h"

#include "WindowTest.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);

    WindowTest windowTest("WindowTest");
    windowTest.Create("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    for (const std::string& path : StrSplit(app.GetEnv("PATH"), ";", true))
    {
        LogPrint("Global", LogLevel::Info, "PATH=%s", path.c_str());
    }

    return app.Run();
}
