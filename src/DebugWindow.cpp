#include "DebugWindow.h"

#include <iostream>
#include <sstream>


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
    if (!CreateDeviceWGL(hwnd, &m_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &m_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    wglMakeCurrent(m_MainWindow.hDC, m_MainWindow.hRC);

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

    CleanupDeviceWGL(hwnd, &m_MainWindow);
    wglDeleteContext(m_hRC);
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
        //ImGui::SetWindowSize(ImVec2(m_Width, m_Height));

        for (auto& field : registeredFields) {
            field->draw();
        }

        ImGui::End();
        //--------End IMGUI Window----------//


        // Rendering
        ImGui::Render();
        glViewport(0, 0, m_Width, m_Height);
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
        ::SwapBuffers(m_MainWindow.hDC);

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

    wglMakeCurrent(m_MainWindow.hDC, m_MainWindow.hRC);
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