#include "Gui.h"
#include "rad/Core/Logging.h"

Gui::Gui(Window* window, SDL_Renderer* renderer) :
    m_window(window),
    m_renderer(renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForSDLRenderer(m_window->GetHandle(), m_renderer);
    ImGui_ImplSDLRenderer_Init(m_renderer);

    float dpi = m_window->GetDisplayDPI();
    float fontSize = 12.0f * (dpi / 72.0f);
#ifdef _WIN32
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", fontSize);
#endif
}

Gui::~Gui()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Gui::BeginFrame()
{
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Gui::EndFrame()
{
}

void Gui::Render()
{
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
}
