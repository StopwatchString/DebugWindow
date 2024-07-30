#include "winapiHelpers.h"

HGLRC WGL_WindowData::hRC = {};

PFNWGLSWAPINTERVALEXTPROC_DEBUGWINDOWALIAS wglSwapIntervalEXT_DEBUGWINDOWALIAS = nullptr;

void loadSwapIntervalExtension()
{
    wglSwapIntervalEXT_DEBUGWINDOWALIAS = (PFNWGLSWAPINTERVALEXTPROC_DEBUGWINDOWALIAS)wglGetProcAddress("wglSwapIntervalEXT");
}


bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    data->hDC = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(data->hDC, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(data->hDC, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, data->hDC);

    if (!data->hRC)
        data->hRC = wglCreateContext(data->hDC);
    return true;
}

void CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(nullptr, nullptr);
    ::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            //g_Width = LOWORD(lParam);
            //g_Height = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}


// Support function for multi-viewports
// Unlike most other backend combination, we need specific hooks to combine Win32+OpenGL.
// We could in theory decide to support Win32-specific code in OpenGL backend via e.g. an hypothetical ImGui_ImplOpenGL3_InitForRawWin32().
void Hook_Renderer_CreateWindow(ImGuiViewport* viewport)
{
    assert(viewport->RendererUserData == NULL);

    WGL_WindowData* data = IM_NEW(WGL_WindowData);
    CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
    viewport->RendererUserData = data;
}

void Hook_Renderer_DestroyWindow(ImGuiViewport* viewport)
{
    if (viewport->RendererUserData != NULL)
    {
        WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData;
        CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
        IM_DELETE(data);
        viewport->RendererUserData = NULL;
    }
}

void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*)
{
    // Activate the platform window DC in the OpenGL rendering context
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData) {
        data->m_ReturnDeviceContext = wglGetCurrentDC();
        data->m_ReturnGLRenderContext = wglGetCurrentContext();

        wglMakeCurrent(data->hDC, data->hRC);
        if (!data->vsyncDisabled && wglSwapIntervalEXT_DEBUGWINDOWALIAS != nullptr) {
            wglSwapIntervalEXT_DEBUGWINDOWALIAS(0);
            data->vsyncDisabled = true;
        }
    }
}

void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*)
{
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData) {
        ::SwapBuffers(data->hDC);

        wglMakeCurrent(data->m_ReturnDeviceContext, data->m_ReturnGLRenderContext);
    }
}