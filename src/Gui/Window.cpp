#include "rad/Gui/Window.h"
#include "rad/Gui/Application.h"

#include "rad/Core/Exception.h"
#include "rad/Core/Logging.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

Window::Window()
{
}

Window::~Window()
{
    if (m_handle)
    {
        Destroy();
    }
}

bool Window::Create(const char* title, int x, int y, int width, int height, Uint32 flags)
{
    m_handle = SDL_CreateWindow(title, x, y, width, height, flags);
    if (m_handle)
    {
        m_id = SDL_GetWindowID(m_handle);
        Application::GetInstance()->RegisterEventHandler(this);
        return true;
    }
    return false;
}

void Window::Destroy()
{
    if (m_handle)
    {
        Application::GetInstance()->UnregisterEventHandler(this);
        SDL_DestroyWindow(m_handle);
        m_handle = nullptr;
    }
}

void Window::GetPosition(int* x, int* y)
{
    SDL_GetWindowPosition(m_handle, x, y);
}

void Window::GetSize(int* w, int* h)
{
    SDL_GetWindowSize(m_handle, w, h);
}

void Window::GetMaximumSize(int* w, int* h)
{
    SDL_GetWindowMaximumSize(m_handle, w, h);
}

void Window::GetMinimumSize(int* w, int* h)
{
    SDL_GetWindowMinimumSize(m_handle, w, h);
}

void Window::GetBorderSize(int* top, int* left, int* bottom, int* right)
{
    SDL_GetWindowBordersSize(m_handle, top, left, bottom, right);
}

float Window::GetBrightness()
{
    return SDL_GetWindowBrightness(m_handle);
}

int Window::GetDisplayIndex()
{
    return SDL_GetWindowDisplayIndex(m_handle);
}

int Window::GetDisplayMode(SDL_DisplayMode* displayMode)
{
    return SDL_GetWindowDisplayMode(m_handle, displayMode);
}

float Window::GetDisplayDPI()
{
    int displayIndex = GetDisplayIndex();
    float ddpi = 96.0f;
    float hdpi = 96.0f;
    float vdpi = 96.0f;
    SDL_GetDisplayDPI(displayIndex, &ddpi, &hdpi, &vdpi);
    return ddpi;
}

int Window::GetFlags()
{
    return SDL_GetWindowFlags(m_handle);
}

int Window::GetGammaRamp(Uint16* r, Uint16* g, Uint16* b)
{
    return SDL_GetWindowGammaRamp(m_handle, r, g, b);
}

bool Window::IsGrabbed()
{
    return (SDL_GetWindowGrab(m_handle) == SDL_TRUE);
}

float Window::GetOpacity()
{
    float opacity = 1.0f;
    SDL_GetWindowOpacity(m_handle, &opacity);
    return opacity;
}

Uint32 Window::GetPixelFormat()
{
    return SDL_GetWindowPixelFormat(m_handle);
}

const char* Window::GetTitle()
{
    return SDL_GetWindowTitle(m_handle);
}


void Window::Resize(int w, int h)
{
    SDL_SetWindowSize(m_handle, w, h);
}

void Window::Hide()
{
    SDL_HideWindow(m_handle);
}

void Window::Show()
{
    SDL_ShowWindow(m_handle);
}

void Window::Maximize()
{
    SDL_MaximizeWindow(m_handle);
}

void Window::Minimize()
{
    SDL_MinimizeWindow(m_handle);
}

void Window::Raise()
{
    SDL_RaiseWindow(m_handle);
}

void Window::Restore()
{
    SDL_RestoreWindow(m_handle);
}

void Window::SetBordered(bool bordered)
{
    SDL_SetWindowBordered(m_handle, bordered ? SDL_TRUE : SDL_FALSE);
}

int Window::SetBrightness(float brightness)
{
    return SDL_SetWindowBrightness(m_handle, brightness);
}

int Window::SetDisplayMode(const SDL_DisplayMode& mode)
{
    return SDL_SetWindowDisplayMode(m_handle, &mode);
}

int Window::SetGammaRamp(const Uint16* r, const Uint16* g, const Uint16* b)
{
    return SDL_SetWindowGammaRamp(m_handle, r, g, b);
}

void Window::Grab(bool grabbed)
{
    SDL_SetWindowGrab(m_handle, grabbed ? SDL_TRUE : SDL_FALSE);
}

void Window::SetIcon(SDL_Surface* icon)
{
    SDL_SetWindowIcon(m_handle, icon);
}

void Window::SetInputFocus()
{
    SDL_SetWindowInputFocus(m_handle);
}

void Window::SetMaximumSize(int w, int h)
{
    SDL_SetWindowMaximumSize(m_handle, w, h);
}

void Window::SetMinimumSize(int w, int h)
{
    SDL_SetWindowMinimumSize(m_handle, w, h);
}

void Window::SetTitle(std::string_view title)
{
    SDL_SetWindowTitle(m_handle, title.data());
}

bool Window::OnEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_WINDOWEVENT:
        if (m_id == event.window.windowID)
        {
            OnWindowEvent(event.window);
            return true;
        }
        break;
    case SDL_KEYDOWN:
        if (m_id == event.key.windowID)
        {
            OnKeyDown(event.key);
            return true;
        }
        break;
    case SDL_KEYUP:
        if (m_id == event.key.windowID)
        {
            OnKeyUp(event.key);
            return true;
        }
        break;
    case SDL_TEXTEDITING:
        if (m_id == event.edit.windowID)
        {
            OnTextEditing(event.edit);
            return true;
        }
        break;
    case SDL_TEXTINPUT:
        if (m_id == event.text.windowID)
        {
            OnTextInput(event.text);
            return true;
        }
        break;
    case SDL_MOUSEMOTION:
        if (m_id == event.motion.windowID)
        {
            OnMouseMove(event.motion);
            return true;
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (m_id == event.button.windowID)
        {
            OnMouseButtonDown(event.button);
            return true;
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (m_id == event.button.windowID)
        {
            OnMouseButtonUp(event.button);
            return true;
        }
        break;
    case SDL_MOUSEWHEEL:
        if (m_id == event.wheel.windowID)
        {
            OnMouseWheel(event.wheel);
            return true;
        }
        break;
    }
    return false;
}

void Window::OnWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.event)
    {
    case SDL_WINDOWEVENT_SHOWN:
        //window has been shown
        OnShown();
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        // window has been hidden
        OnHidden();
        break;
    case SDL_WINDOWEVENT_EXPOSED:
        // window has been exposedand should be redrawn
        OnExposed();
        break;
    case SDL_WINDOWEVENT_MOVED:
        // window has been moved to data1, data2
        OnMoved(windowEvent.data1, windowEvent.data2);
        break;
    case SDL_WINDOWEVENT_RESIZED:
        // window has been resized to data1xdata2; this event is always preceded by SDL_WINDOWEVENT_SIZE_CHANGED
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        // window size has changed, either as a result of an API call or through the system or user changing the window size; this event is followed by SDL_WINDOWEVENT_RESIZED if the size was changed by an external event, i.e.the user or the window manager
        OnResized(windowEvent.data1, windowEvent.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED:
        // window has been minimized
        OnMinimized();
        break;
    case SDL_WINDOWEVENT_MAXIMIZED:
        // window has been maximized
        OnMaximized();
        break;
    case SDL_WINDOWEVENT_RESTORED:
        // window has been restored to normal size and position
        OnRestored();
        break;
    case SDL_WINDOWEVENT_ENTER:
        // window has gained mouse focus
        OnEnter();
        break;
    case SDL_WINDOWEVENT_LEAVE:
        // window has lost mouse focus
        OnLeave();
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        // window has gained keyboard focus
        OnFocusGained();
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        // window has lost keyboard focus
        OnFocusLost();
        break;
    case SDL_WINDOWEVENT_CLOSE:
        // the window manager requests that the window be closed
        OnClose();
        break;
    }
}

void Window::OnClose()
{
    m_shouldClose = true;
    Destroy();
}
