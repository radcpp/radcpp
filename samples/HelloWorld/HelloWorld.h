#pragma once

#include "rad/Gui/Application.h"
#include "rad/Gui/Window.h"

#include "Gui.h"

class EnvWindow;

class HelloWorld : public Window
{
public:
    HelloWorld();
    ~HelloWorld();

    bool Init();

private:
    virtual bool OnEvent(const SDL_Event& event);
    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown);
    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp);

    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion) {}
    virtual void OnMouseButtonDown(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseButtonUp(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseWheel(const SDL_MouseWheelEvent& mouseWheel) {}

    virtual void OnRender();

    SDL_Renderer* m_renderer = nullptr;
    Ref<Gui> m_gui;
    bool m_showDemoWindow = true;
    bool m_showEnvironment = true;
    Ref<EnvWindow> m_envWindow;

}; // class HelloWorld
