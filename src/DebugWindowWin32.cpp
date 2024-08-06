#include "DebugWindowWin32.h"

HGLRC WGL_WindowData::hRC = {};
bool WGL_WindowData::vsyncEnabled = false;

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

//---------------------------------------------------------
// Hook_Renderer_CreateWindow()
//---------------------------------------------------------
void Hook_Renderer_CreateWindow(ImGuiViewport* viewport)
{
    assert(viewport->RendererUserData == NULL);

    WGL_WindowData* data = IM_NEW(WGL_WindowData);
    CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
    viewport->RendererUserData = data;
}

//---------------------------------------------------------
// Hook_Renderer_DestroyWindow()
//---------------------------------------------------------
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

//---------------------------------------------------------
// Hook_Platform_RenderWindow()
//---------------------------------------------------------
void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*)
{
    // Activate the platform window DC in the OpenGL rendering context
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData) {
        data->m_ReturnDeviceContext = wglGetCurrentDC();
        data->m_ReturnGLRenderContext = wglGetCurrentContext();

        wglMakeCurrent(data->hDC, data->hRC);
        if (!data->vsyncEnabled && wglSwapIntervalEXT_DEBUGWINDOWALIAS != nullptr) {
            wglSwapIntervalEXT_DEBUGWINDOWALIAS(1);
        }
        else {
            wglSwapIntervalEXT_DEBUGWINDOWALIAS(0);
        }
    }
}

//---------------------------------------------------------
// Hook_Renderer_SwapBuffers()
//---------------------------------------------------------
void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*)
{
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData) {
        ::SwapBuffers(data->hDC);

        wglMakeCurrent(data->m_ReturnDeviceContext, data->m_ReturnGLRenderContext);
    }
}

//---------------------------------------------------------
// Constructor()
//---------------------------------------------------------
DebugWindowWin32::DebugWindowWin32()
{
    init();
}

//---------------------------------------------------------
// Destructor()
//---------------------------------------------------------
DebugWindowWin32::~DebugWindowWin32()
{
    if (m_Open) {
        cleanup();
        m_Open = false;
    }
}

//---------------------------------------------------------
// init()
//---------------------------------------------------------
void DebugWindowWin32::init()
{
    pushOpenGLState();

    // Create application window
    ImGui_ImplWin32_EnableDpiAwareness();
    m_WindowClass = { sizeof(m_WindowClass), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Debug Window", nullptr };
    ::RegisterClassExW(&m_WindowClass);

    const char* text = OS_WINDOW_NAME;
    wchar_t wtext[20];
    mbstowcs(wtext, text, strlen(text) + 1);//Plus null
    LPWSTR ptr = wtext;

    m_WindowHandle = ::CreateWindowW(m_WindowClass.lpszClassName, ptr, WS_POPUPWINDOW, 0, 0, 0, 0, nullptr, nullptr, m_WindowClass.hInstance, nullptr);

    // Initialize OpenGL
    if (CreateDeviceWGL(m_WindowHandle, &m_MainWindow))
    {
        wglMakeCurrent(m_MainWindow.hDC, m_MainWindow.hRC);

        // Load an alias of wglSwapIntervalEXT so we can disable vsync, which imposes additional limits on our execution time.
        // This needs to be done both for this primary context and any created by the hooks below for viewports.
        loadSwapIntervalExtension();
        if (wglSwapIntervalEXT_DEBUGWINDOWALIAS != nullptr) {
            wglSwapIntervalEXT_DEBUGWINDOWALIAS(0);
        }



        // Show the window
        ::ShowWindow(m_WindowHandle, SW_SHOWDEFAULT);
        ::UpdateWindow(m_WindowHandle);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();

        // Get IO and config
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows
        io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        if (!m_PlatformBackendsInit) {
            ImGui_ImplWin32_InitForOpenGL(m_WindowHandle);
            ImGui_ImplOpenGL3_Init();

            // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
                IM_ASSERT(platform_io.Renderer_CreateWindow == NULL);
                IM_ASSERT(platform_io.Renderer_DestroyWindow == NULL);
                IM_ASSERT(platform_io.Renderer_SwapBuffers == NULL);
                IM_ASSERT(platform_io.Platform_RenderWindow == NULL);
                platform_io.Renderer_CreateWindow = Hook_Renderer_CreateWindow;
                platform_io.Renderer_DestroyWindow = Hook_Renderer_DestroyWindow;
                platform_io.Renderer_SwapBuffers = Hook_Renderer_SwapBuffers;
                platform_io.Platform_RenderWindow = Hook_Platform_RenderWindow;
            }
            m_PlatformBackendsInit = true;
        }

        m_Open = true;
    }
    else {
        CleanupDeviceWGL(m_WindowHandle, &m_MainWindow);
        ::DestroyWindow(m_WindowHandle);
        ::UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);
    }

    popOpenGLState();
}

//---------------------------------------------------------
// cleanup()
//---------------------------------------------------------
void DebugWindowWin32::cleanup()
{
    pushOpenGLState();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    CleanupDeviceWGL(m_WindowHandle, &m_MainWindow);
    wglDeleteContext(m_MainWindow.hRC);
    ::DestroyWindow(m_WindowHandle);
    ::UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);

    popOpenGLState();
}

//---------------------------------------------------------
// drawImpl()
//---------------------------------------------------------
void DebugWindowWin32::drawImpl()
{
    pushOpenGLState();

    // Poll and handle messages (inputs, window resize, etc.)
    // See the WndProc() function below for our to dispatch events to the Win32 backend.
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT) {
            closeWindow();
            return;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();

    drawWindow();

    glViewport(0, 0, m_Width, m_Height);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    ::SwapBuffers(m_MainWindow.hDC);

    popOpenGLState();
}

//---------------------------------------------------------
// toggleVsync()
//---------------------------------------------------------
void DebugWindowWin32::toggleVsync()
{
    if (m_VsyncEnabled) {
        wglSwapIntervalEXT_DEBUGWINDOWALIAS(1);
        m_MainWindow.vsyncEnabled = true;
    }
    else {
        wglSwapIntervalEXT_DEBUGWINDOWALIAS(0);
        m_MainWindow.vsyncEnabled = false;
    }
}

//---------------------------------------------------------
// pushOpenGLState()
//---------------------------------------------------------
void DebugWindowWin32::pushOpenGLState()
{
    m_ReturnOpenGLContext = wglGetCurrentContext();
    m_ReturnOpenGLDeviceContext = wglGetCurrentDC();

    wglMakeCurrent(m_MainWindow.hDC, m_MainWindow.hRC);
}

//---------------------------------------------------------
// popOpenGLState()
//---------------------------------------------------------
void DebugWindowWin32::popOpenGLState()
{
    wglMakeCurrent(m_ReturnOpenGLDeviceContext, m_ReturnOpenGLContext);
}