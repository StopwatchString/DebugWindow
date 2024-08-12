#include "DebugWindow.h"

#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <gl/GL.h>

// Data stored per platform window
struct WGL_WindowData {
    HDC hDC;
    static HGLRC hRC;
    static bool vsyncEnabled;
    HDC m_ReturnDeviceContext;
    HGLRC m_ReturnGLRenderContext;
};

typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC_DEBUGWINDOWALIAS)(int interval);
extern PFNWGLSWAPINTERVALEXTPROC_DEBUGWINDOWALIAS wglSwapIntervalEXT_DEBUGWINDOWALIAS;

void loadSwapIntervalExtension();
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data);
void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Hook_Renderer_CreateWindow(ImGuiViewport* viewport);
void Hook_Renderer_DestroyWindow(ImGuiViewport* viewport);
void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*);
void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*);

class DebugWindowWin32 : public DebugWindow
{
public:
    DebugWindowWin32();
    ~DebugWindowWin32();

protected:
    virtual void init() override;
    virtual void cleanup() override;
    virtual void drawImpl() override;
    virtual void toggleVsync() override;
    virtual void pushOpenGLState() override;
    virtual void popOpenGLState() override;

private:

    // OpenGL State Management
    HGLRC               m_ReturnOpenGLContext{};
    HDC                 m_ReturnOpenGLDeviceContext{};
    // Win32 Window Management
    WNDCLASSEXW         m_WindowClass{};
    HWND                m_WindowHandle{};
    WGL_WindowData      m_MainWindow{};
};