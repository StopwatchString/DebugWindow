/*
 Class:          DebugWindowWin32
 Author:         Mason Speck

  Implementation of DebugWindow class but using the Win32 API
  directly to create an OpenGL context and window.

  This implementation is a little messy and has weird behavior.
  DearImgui recommends against relying on the raw Win32 API and
  this is borne out by occasional issues like viewport size within
  the window being incorrect, or rarely a crash when interacting
  with the GUI.

  The one benefit to using the Win32 API directly is maximal compatibility
  when adding to codebases that are older. DearImgui does a great job at
  aliasing all the OpenGL functions it imports, and the Win32 API calls
  won't break due to old windows header incompatibility because they're
  provided by the platform and not versioned into the projects that use them.

  A lot of the Win32 specific code was taken directly from the DearImgui example,
  with some modifications by me to allow for toggling Vsync on viewports.

  Known issues
    -Vsync settings are sometimes ignored
    -Rare random crashes have been observed while interacting with the GUI
 */

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
    virtual void implementationDrawWrapper() override;
    virtual void toggleVsync() override;
    virtual void pushOpenGLContext() override;
    virtual void popOpenGLContext() override;

private:

    // OpenGL State Management
    HGLRC               m_ReturnOpenGLContext{};
    HDC                 m_ReturnOpenGLDeviceContext{};
    // Win32 Window Management
    WNDCLASSEXW         m_WindowClass{};
    HWND                m_WindowHandle{};
    WGL_WindowData      m_MainWindow{};
};