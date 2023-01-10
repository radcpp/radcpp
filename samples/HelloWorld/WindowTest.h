#pragma once

#include "rad/Gui/Application.h"
#include "rad/Gui/Window.h"

#include "rad/Core/Logging.h"

class WindowTest : public Window
{
public:
    WindowTest(std::string_view name) :
        m_name(name)
    {
        LogPrint(m_name.c_str(), LogLevel::Info, __FUNCTION__);

        float windowScale = GetDisplayDPI() / 96.0f;
        Create("Window Test",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            int(1600 * windowScale), int(900 * windowScale),
            SDL_WINDOW_ALLOW_HIGHDPI);
    }

    ~WindowTest()
    {
        LogPrint(m_name.c_str(), LogLevel::Info, __FUNCTION__);
    }

    virtual void OnShown() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnShown");
    }

    virtual void OnHidden() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnHidden");
    }

    virtual void OnExposed() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnExposed");
    }

    virtual void OnMoved(int x, int y) override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnMoved");
    }

    virtual void OnResized(int width, int height) override
    {
        Window::OnResized(width, height);
        LogPrint(m_name.c_str(), LogLevel::Info, "OnResized");
    }

    virtual void OnMinimized() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnMinimized");
    }

    virtual void OnMaximized() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnMaximized");
    }

    virtual void OnRestored() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnRestored");
    }

    virtual void OnEnter() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnEnter");
    }

    virtual void OnLeave() override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnLeave");
    }

    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown) override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnKeyDown: %s", SDL_GetKeyName(keyDown.keysym.sym));
    }

    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp) override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnKeyUp: %s", SDL_GetKeyName(keyUp.keysym.sym));
    }

    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion) override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnMouseMove: Pos=(%4d, %4d)", mouseMotion.x, mouseMotion.y);
    }

    virtual void OnMouseButtonDown(const SDL_MouseButtonEvent& mouseButton) override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnMouseButtonDown: %s", GetMouseButtonName(mouseButton.button));
    }

    virtual void OnMouseButtonUp(const SDL_MouseButtonEvent& mouseButton) override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnMouseButtonUp: %s", GetMouseButtonName(mouseButton.button));
    }

    virtual void OnMouseWheel(const SDL_MouseWheelEvent& mouseWheel) override
    {
        LogPrint(m_name.c_str(), LogLevel::Info, "OnMouseWheel: %+d", mouseWheel.y);
    }

private:
    std::string m_name;

    const char* GetMouseButtonName(Uint8 button)
    {
        switch (button)
        {
        case SDL_BUTTON_LEFT: return "SDL_BUTTON_LEFT";
        case SDL_BUTTON_MIDDLE: return "SDL_BUTTON_MIDDLE";
        case SDL_BUTTON_RIGHT: return "SDL_BUTTON_RIGHT";
        case SDL_BUTTON_X1: return "SDL_BUTTON_X1";
        case SDL_BUTTON_X2: return "SDL_BUTTON_X2";
        }
        return "SDL_BUTTON_UNKNOWN";
    }

}; // class WindowTest
