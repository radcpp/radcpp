#pragma once

#include "Gui.h"

class GuiDemo : public Window
{
public:
    GuiDemo();
    ~GuiDemo();

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

}; // class GuiDemo
