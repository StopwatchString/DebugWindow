#include "DebugWindow.h"
#include <iostream>
#include <sstream>

// Data stored per platform window
struct WGL_WindowData { HDC hDC; };

// Data
static HGLRC            g_hRC;
static WGL_WindowData   g_MainWindow;
static int              g_Width;
static int              g_Height;

// Helper functions
bool CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
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
            g_Width = LOWORD(lParam);
            g_Height = HIWORD(lParam);
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
static void Hook_Renderer_CreateWindow(ImGuiViewport* viewport)
{
    assert(viewport->RendererUserData == NULL);

    WGL_WindowData* data = IM_NEW(WGL_WindowData);
    CreateDeviceWGL((HWND)viewport->PlatformHandle, data);
    viewport->RendererUserData = data;
}

static void Hook_Renderer_DestroyWindow(ImGuiViewport* viewport)
{
    if (viewport->RendererUserData != NULL)
    {
        WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData;
        CleanupDeviceWGL((HWND)viewport->PlatformHandle, data);
        IM_DELETE(data);
        viewport->RendererUserData = NULL;
    }
}

static void Hook_Platform_RenderWindow(ImGuiViewport* viewport, void*)
{
    // Activate the platform window DC in the OpenGL rendering context
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
        wglMakeCurrent(data->hDC, g_hRC);
}

static void Hook_Renderer_SwapBuffers(ImGuiViewport* viewport, void*)
{
    if (WGL_WindowData* data = (WGL_WindowData*)viewport->RendererUserData)
        ::SwapBuffers(data->hDC);
}

//---------------------------------------------------------
// DebugWindow()
//---------------------------------------------------------
DebugWindow::DebugWindow()
{

}

//---------------------------------------------------------
// ~DebugWindow()
//---------------------------------------------------------
DebugWindow::~DebugWindow()
{
    cleanup();
}

//---------------------------------------------------------
// init()
//---------------------------------------------------------
bool DebugWindow::init()
{
    pushOpenGLState();

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    wc = { sizeof(wc), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Debug Window", nullptr };
    ::RegisterClassExW(&wc);
    hwnd = ::CreateWindowW(wc.lpszClassName, L"Debug Window", WS_POPUPWINDOW, 100, 100, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize OpenGL
    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();

    // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
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

    initialized = true;

    popOpenGLState();
    return initialized;
}

void DebugWindow::cleanup()
{
    pushOpenGLState();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    CleanupDeviceWGL(hwnd, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    popOpenGLState();
}

//---------------------------------------------------------
// draw()
//---------------------------------------------------------
void DebugWindow::draw()
{
    pushOpenGLState();

    if (initialized) {

        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                exited = true;
                return;
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //--------Begin IMGUI Window--------//
        ImGui::Begin("Debug Panel");
        //ImGui::SetWindowPos(ImVec2(0, 0));
        //ImGui::SetWindowSize(ImVec2(g_Width, g_Height));

        for (auto& field : registeredFields) {
            field->draw();
        }

        ImGui::End();
        //--------End IMGUI Window----------//


        // Rendering
        ImGui::Render();
        glViewport(0, 0, g_Width, g_Height);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            // TODO for OpenGL: restore current GL context.
        }

        // Present
        ::SwapBuffers(g_MainWindow.hDC);

    }
    else {
        std::cout << "DebugWindow::draw() Function called but 'initialized' is false." << '\n';
    }

    popOpenGLState();
}

//---------------------------------------------------------
// addSliderFloat()
//---------------------------------------------------------
void DebugWindow::addSliderFloat(const char* label, float& f, float lowerBound, float upperBound)
{
    std::string registeredLabel = registerAndGetLabel(label);
    registeredFields.emplace_back(std::make_unique<SliderFloat>(registeredLabel, f, lowerBound, upperBound));
}

//---------------------------------------------------------
// addInputText()
//---------------------------------------------------------
void DebugWindow::addInputText(const char* label, char* buf, size_t bufSize)
{
    std::string registeredLabel = registerAndGetLabel(label);
    registeredFields.emplace_back(std::make_unique<InputText>(registeredLabel, buf, bufSize));
}

//---------------------------------------------------------
// addButton()
//---------------------------------------------------------
void DebugWindow::addButton(const char* label, std::function<void(void)> callback)
{
    std::string registeredLabel = registerAndGetLabel(label);
    registeredFields.emplace_back(std::make_unique<Button>(registeredLabel, callback));
}

//---------------------------------------------------------
// addButton()
//---------------------------------------------------------
void DebugWindow::addPlotLine(const char* label, std::vector<float>& data)
{
    std::string registeredLabel = registerAndGetLabel(label);
    registeredFields.emplace_back(std::make_unique<PlotLine>(registeredLabel, data));
}

//---------------------------------------------------------
// pushOpenGLState()
//---------------------------------------------------------
void DebugWindow::pushOpenGLState()
{
    m_returnOpenGLContext = wglGetCurrentContext();
    m_returnOpenGLDeviceContext = wglGetCurrentDC();

    wglMakeCurrent(g_MainWindow.hDC, g_hRC);
}

//---------------------------------------------------------
// popOpenGLState()
//---------------------------------------------------------
void DebugWindow::popOpenGLState()
{
    wglMakeCurrent(m_returnOpenGLDeviceContext, m_returnOpenGLContext);
}

//---------------------------------------------------------
// scaleUI()
//---------------------------------------------------------
void DebugWindow::scaleUI(float scale_factor) {
    ImGuiStyle& style = ImGui::GetStyle();

    // Scale the font
    io->FontGlobalScale = scale_factor;

    // Scale all sizes in the style
    style.ScaleAllSizes(scale_factor);
}

//---------------------------------------------------------
// registerAndGetLabel()
//---------------------------------------------------------
std::string DebugWindow::registerAndGetLabel(const char* label)
{
    std::string strLabel(label);
    if (registeredLabels.find(strLabel) != registeredLabels.end()) {
        registeredLabels[strLabel]++;
        int num = registeredLabels[strLabel];
        strLabel.append(" (");
        strLabel.append(std::to_string(num));
        strLabel.append(")");
    }
    else {
        registeredLabels[strLabel] = 0;
    }

    return strLabel;
}