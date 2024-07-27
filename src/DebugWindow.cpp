#include "DebugWindow.h"

#include <iostream>
#include <sstream>

bool DebugWindow::m_PlatformBackendsInit = false;

//---------------------------------------------------------
// DebugWindow()
//---------------------------------------------------------
DebugWindow::DebugWindow()
{
    init();
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
    m_WindowClass = { sizeof(m_WindowClass), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Debug Window", nullptr };
    ::RegisterClassExW(&m_WindowClass);
    m_WindowHandle = ::CreateWindowW(m_WindowClass.lpszClassName, L"Debug Window", WS_POPUPWINDOW, 100, 100, 0, 0, nullptr, nullptr, m_WindowClass.hInstance, nullptr);

    // Initialize OpenGL
    if (!CreateDeviceWGL(m_WindowHandle, &m_MainWindow))
    {
        CleanupDeviceWGL(m_WindowHandle, &m_MainWindow);
        ::DestroyWindow(m_WindowHandle);
        ::UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);
        return 1;
    }

    wglMakeCurrent(m_MainWindow.hDC, m_MainWindow.hRC);

    // Show the window
    ::ShowWindow(m_WindowHandle, SW_SHOWDEFAULT);
    ::UpdateWindow(m_WindowHandle);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    
    // Get IO and config
    m_ImguiIo = &ImGui::GetIO();
    m_ImguiIo->ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
    m_ImguiIo->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows

    // Get Style
    m_ImguiStyle = &ImGui::GetStyle();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    if (!m_PlatformBackendsInit) {
        ImGui_ImplWin32_InitForOpenGL(m_WindowHandle);
        ImGui_ImplOpenGL3_Init();
        
        // Win32+GL needs specific hooks for viewport, as there are specific things needed to tie Win32 and GL api.
        if (m_ImguiIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
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
    popOpenGLState();
    return m_Open;
}

//---------------------------------------------------------
// cleanup()
//---------------------------------------------------------
void DebugWindow::cleanup()
{
    if (isWindowOpen()) {
        pushOpenGLState();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        CleanupDeviceWGL(m_WindowHandle, &m_MainWindow);
        wglDeleteContext(m_HandleRenderContext);
        ::DestroyWindow(m_WindowHandle);
        ::UnregisterClassW(m_WindowClass.lpszClassName, m_WindowClass.hInstance);

        popOpenGLState();
    }
}

//---------------------------------------------------------
// draw()
//---------------------------------------------------------
void DebugWindow::draw()
{
    if (isWindowOpen()) {
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
        ImGui::NewFrame();

        ImGui::Begin("Debug Panel", nullptr, ImGuiWindowFlags_NoCollapse);
        //ImGui::SetWindowPos(ImVec2(0, 0));
        ImGui::SetWindowSize(ImVec2(m_Width, m_Height));
        for (const auto& drawable : m_Drawables) {
            drawable();
        }
        ImGui::End();

        ImGui::Render();

        glViewport(0, 0, m_Width, m_Height);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (m_ImguiIo->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        ::SwapBuffers(m_MainWindow.hDC);

        popOpenGLState();
    }
    else {
        std::cout << "DebugWindow::draw() Window isn't open." << '\n';
    }

}

//---------------------------------------------------------
// addSliderFloat()
//---------------------------------------------------------
void DebugWindow::addSliderFloat(const char* label, float& f, float lowerBound, float upperBound)
{
    std::string registeredLabel = registerAndGetLabel(label);

    m_Drawables.emplace_back([registeredLabel, &f, lowerBound, upperBound]() {
        ImGui::SliderFloat(registeredLabel.c_str(), &f, lowerBound, upperBound);
    });
}

//---------------------------------------------------------
// addInputText()
//---------------------------------------------------------
void DebugWindow::addInputText(const char* label, char* buf, size_t bufSize)
{
    std::string registeredLabel = registerAndGetLabel(label);
    m_Drawables.emplace_back([registeredLabel, buf, bufSize]() {
        ImGui::InputText(registeredLabel.c_str(), buf, bufSize);
    });
}

//---------------------------------------------------------
// addButton()
//---------------------------------------------------------
void DebugWindow::addButton(const char* label, std::function<void(void)> callback)
{
    std::string registeredLabel = registerAndGetLabel(label);
    m_Drawables.emplace_back([registeredLabel, callback]() {
        if (ImGui::Button(registeredLabel.c_str()))
            callback();
    });
}

//---------------------------------------------------------
// addButton()
//---------------------------------------------------------
void DebugWindow::addPlotLine(const char* label, std::vector<float>& data)
{
    std::string registeredLabel = registerAndGetLabel(label);
    m_Drawables.emplace_back([registeredLabel, &data]() {
        if (ImPlot::BeginPlot(registeredLabel.c_str())) {
            ImPlot::PlotLine("My Line Plot", data.data(), data.size());
            ImPlot::EndPlot();
        }
    });
}

//---------------------------------------------------------
// pushOpenGLState()
//---------------------------------------------------------
void DebugWindow::pushOpenGLState()
{
    m_ReturnOpenGLContext = wglGetCurrentContext();
    m_ReturnOpenGLDeviceContext = wglGetCurrentDC();

    wglMakeCurrent(m_MainWindow.hDC, m_MainWindow.hRC);
}

//---------------------------------------------------------
// popOpenGLState()
//---------------------------------------------------------
void DebugWindow::popOpenGLState()
{
    wglMakeCurrent(m_ReturnOpenGLDeviceContext, m_ReturnOpenGLContext);
}

//---------------------------------------------------------
// registerAndGetLabel()
//---------------------------------------------------------
std::string DebugWindow::registerAndGetLabel(std::string label)
{
    std::string strLabel(label);
    unsigned int counter = 0;
    while (m_RegisteredLabels.find(strLabel) != m_RegisteredLabels.end()) {
        ++counter;
        strLabel = label;
        strLabel.append(" (").append(std::to_string(counter)).append(")");
    }
    m_RegisteredLabels.insert(strLabel);

    return strLabel;
}