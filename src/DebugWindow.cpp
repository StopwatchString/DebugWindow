#include "DebugWindow.h"

#include <iostream>
#include <sstream>

bool DebugWindow::m_PlatformBackendsInit = false;

//---------------------------------------------------------
// DebugWindow()
//---------------------------------------------------------
DebugWindow::DebugWindow(uint32_t width, uint32_t height) : m_Width(width), m_Height(height)
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
void DebugWindow::init()
{
    pushOpenGLState();

    // Create application window
    ImGui_ImplWin32_EnableDpiAwareness();
    m_WindowClass = { sizeof(m_WindowClass), CS_OWNDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Debug Window", nullptr };
    ::RegisterClassExW(&m_WindowClass);
    m_WindowHandle = ::CreateWindowW(m_WindowClass.lpszClassName, L"Debug Window", WS_POPUPWINDOW, 100, 100, 0, 0, nullptr, nullptr, m_WindowClass.hInstance, nullptr);

    // Initialize OpenGL
    if (CreateDeviceWGL(m_WindowHandle, &m_MainWindow))
    {
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
        
        std::cout << m_Width << " " << m_Height << std::endl;
        ImVec2 currSize = ImGui::GetWindowSize();
        if (currSize.x < m_Width || currSize.y < m_Height) {
            ImGui::SetWindowSize(ImVec2(m_Width, m_Height));
        }

        for (const ImguiField field : m_Drawables) {
            field.drawable();
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
void DebugWindow::addSliderFloat(std::string label, float& f, float lowerBound, float upperBound)
{
    ImguiField field;
    field.label = label;
    field.drawable = [label, &f, lowerBound, upperBound]() {
        ImGui::SliderFloat(label.c_str(), &f, lowerBound, upperBound);
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// addInputText()
//---------------------------------------------------------
void DebugWindow::addInputText(std::string label, char* buf, size_t bufSize)
{
    ImguiField field;
    field.label = label;
    field.drawable = [label, buf, bufSize]() {
        ImGui::InputText(label.c_str(), buf, bufSize);
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// addButton()
//---------------------------------------------------------
void DebugWindow::addButton(std::string label, std::function<void(void)> callback)
{
    ImguiField field;
    field.label = label;
    field.drawable = [label, callback]() {
        if (ImGui::Button(label.c_str()))
            callback();
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// addInternalPlot()
//---------------------------------------------------------
void DebugWindow::addInternalPlot(std::string label, uint32_t pointCount)
{
    for (uint32_t i = 0; i < pointCount; ++i) {
        m_InternalPlotData[label].push_back(0);
    }
    std::vector<float>& internalPlot = m_InternalPlotData[label];

    ImguiField field;
    field.label = label;
    field.drawable = [label, &internalPlot]() {
        if (ImPlot::BeginPlot(label.c_str())) {
            ImPlot::PlotLine("Internal Plot", internalPlot.data(), internalPlot.size());
            ImPlot::EndPlot();
        }
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// pushToInternalPlot()
//---------------------------------------------------------
void DebugWindow::pushToInternalPlot(std::string label, float f)
{
    if (m_InternalPlotData.find(label) != m_InternalPlotData.end()) {
        std::vector<float>& internalPlot = m_InternalPlotData[label];
        internalPlot.erase(internalPlot.begin());
        internalPlot.emplace_back(f);
    }
    else {
        std::cout << "DebugWindow::pushToInternalPlot() Label \"" << label << "\" not found in internal plot list." << '\n';
    }
}

//---------------------------------------------------------
// addExternalPlot()
//---------------------------------------------------------
void DebugWindow::addExternalPlot(std::string label, std::vector<float>& data)
{
    ImguiField field;
    field.label = label;
    field.drawable = [label, &data]() {
        if (ImPlot::BeginPlot(label.c_str())) {
            ImPlot::PlotLine("External Plot", data.data(), data.size());
            ImPlot::EndPlot();
        }
    };
    m_Drawables.push_back(field);
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
// registerLabel()
//---------------------------------------------------------
void DebugWindow::registerLabel(std::string& label)
{
    unsigned int counter = 0;
    while (m_RegisteredLabels.find(label) != m_RegisteredLabels.end()) {
        ++counter;
        label = label;
        label.append(" (").append(std::to_string(counter)).append(")");
    }
    m_RegisteredLabels.insert(label);
}