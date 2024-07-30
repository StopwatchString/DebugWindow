#include "DebugWindow.h"

#include "logo.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

        // Load an alias of wglSwapIntervalEXT so we can disable vsync, which imposes additional limits on our execution time.
        // This needs to be done both for this primary context and any created by the hooks below for viewports.
        loadSwapIntervalExtension();
        if (wglSwapIntervalEXT_DEBUGWINDOWALIAS != nullptr) {
            wglSwapIntervalEXT_DEBUGWINDOWALIAS(0);
        }

        loadBackgroundTexture();

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
    if (m_Open) {
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
}

//---------------------------------------------------------
// draw()
//---------------------------------------------------------
void DebugWindow::draw()
{
    if (m_Open) {
        m_TimeDrawStart = std::chrono::steady_clock::now();

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

        ImVec2 currPos = ImGui::GetWindowPos();
        ImVec2 currSize = ImGui::GetWindowSize();

        if (m_BackgroundTextureHandle != 0) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddImage((void*)(intptr_t)m_BackgroundTextureHandle, currPos, ImVec2(currPos.x + currSize.x, currPos.y + currSize.y));
        }

        for (const Drawable drawable : m_Drawables) {
            if (drawable.visible) {
                drawable.draw();
            }
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

        m_TimeDrawEnd = std::chrono::steady_clock::now();
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
    uint32_t id = getNextId();
    Drawable field;
    field.label = label + "##" + std::to_string(id);
    field.draw = [label, id, &f, lowerBound, upperBound]() {
        ImGui::PushID(id);

        ImGui::SliderFloat(label.c_str(), &f, lowerBound, upperBound);

        ImGui::PopID();
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// addInputText()
//---------------------------------------------------------
void DebugWindow::addInputText(std::string label, std::string& input)
{
    uint32_t id = getNextId();
    Drawable field;
    field.label = label + "##" + std::to_string(id);;
    std::array<char, MAX_STRING_INPUT_SIZE>& buf = m_InputStringData[label];
    field.draw = [label, id, &input, &buf]() {
        ImGui::PushID(id);
        
        ImGui::InputText(label.c_str(), buf.data(), MAX_STRING_INPUT_SIZE);
        input = buf.data();

        ImGui::PopID();
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// addButton()
//---------------------------------------------------------
void DebugWindow::addButton(std::string label, std::function<void(void)> callback)
{
    uint32_t id = getNextId();
    Drawable field;
    field.label = label + "##" + std::to_string(id);;
    field.draw = [label, id, callback]() {
        ImGui::PushID(id);

        if (ImGui::Button(label.c_str()))
            callback();

        ImGui::PopID();
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

    uint32_t id = getNextId();
    Drawable field;
    field.label = label + "##" + std::to_string(id);;
    field.draw = [label, id, &internalPlot]() {
        ImGui::PushID(id);
        
        if (ImPlot::BeginPlot(label.c_str())) {
            ImPlot::PlotLine("Internal Plot", internalPlot.data(), internalPlot.size());
            ImPlot::EndPlot();
        }

        ImGui::PopID();
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
    uint32_t id = getNextId();
    Drawable field;
    field.label = label + "##" + std::to_string(id);;
    field.draw = [label, id, &data]() {
        ImGui::PushID(id);
        
        if (ImPlot::BeginPlot(label.c_str())) {
            ImPlot::PlotLine("External Plot", data.data(), data.size());
            ImPlot::EndPlot();
        }

        ImGui::PopID();
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// addSameLine()
//---------------------------------------------------------
void DebugWindow::addSameLine()
{
    uint32_t id = getNextId();
    Drawable field;
    field.label = "Spacing##" + std::to_string(id);
    field.draw = [id]() {
        ImGui::PushID(id);

        ImGui::SameLine();

        ImGui::PopID();
    };
    m_Drawables.push_back(field);
}

//---------------------------------------------------------
// addSpacing()
//---------------------------------------------------------
void DebugWindow::addSpacing(uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t id = getNextId();
        Drawable field;
        field.label = "Spacing##" + std::to_string(id);
        field.draw = [id]() {
            ImGui::PushID(id);

            ImGui::Spacing();

            ImGui::PopID();
        };
        m_Drawables.push_back(field);
    }
}

//---------------------------------------------------------
// setVisibility()
//---------------------------------------------------------
void DebugWindow::setVisibility(std::string label, bool visible)
{
    for (Drawable& field : m_Drawables) {
        if (field.label == label) {
            field.visible = visible;
        }
    }
}

//---------------------------------------------------------
// enableInternalPerformanceStatistics()
//---------------------------------------------------------
void DebugWindow::enableInternalPerformanceStatistics()
{
    if (!m_ShowPerformanceStatistics) {
        std::string& label = m_PerformanceStatisticsID;
        std::vector<double>& startToEndTimings = m_StartToEndTimings;
        std::vector<double>& endToStartTimings = m_EndToStartTimings;
        std::vector<double>& startToEndMinusDrawTimings = m_StartToEndMinusDrawTimings;

        for (uint32_t i = 0; i < 2500; ++i) {
            startToEndTimings.push_back(0);
            endToStartTimings.push_back(0);
            startToEndMinusDrawTimings.push_back(0);
        }

        Drawable field;
        field.label = label;
        field.draw = [&startToEndTimings, &endToStartTimings, &startToEndMinusDrawTimings]() {
            ImPlot::SetNextAxesLimits(0.0, 2500.0, 0.0, 16.6, ImPlotCond_Always);
            if (ImPlot::BeginPlot("Performance")) {
                ImPlot::PlotLine("Start to End", startToEndTimings.data(), startToEndTimings.size(), 1.0, 0.0, ImPlotLineFlags_None);
                ImPlot::PlotLine("End to Start", endToStartTimings.data(), endToStartTimings.size(), 1.0, 0.0, ImPlotLineFlags_None);
                ImPlot::PlotLine("Start to End minus DebugWindow.draw()", startToEndMinusDrawTimings.data(), startToEndMinusDrawTimings.size(), 1.0, 0.0, ImPlotLineFlags_None);
                ImPlot::EndPlot();
            }
        };

        m_Drawables.push_back(field);

        m_ShowPerformanceStatistics = true;
    }
}

//---------------------------------------------------------
// markStartTime()
//---------------------------------------------------------
void DebugWindow::markStartTime()
{
    if (m_ShowPerformanceStatistics) {
        m_TimeMarkStart = std::chrono::steady_clock::now();
        
        std::chrono::steady_clock::duration endToStartMarkDuration = m_TimeMarkStart - m_TimeMarkEnd;
        double endToStartMarkTimeMs = endToStartMarkDuration.count() / 1000000.0;

        m_EndToStartTimings.erase(m_EndToStartTimings.begin());
        m_EndToStartTimings.push_back(endToStartMarkTimeMs);
    }
    else {
        std::cout << "DebugWindow::markStartTime() Performance statistics aren't currently enabled." << std::endl;
    }
}

//---------------------------------------------------------
// markEndTime()
//---------------------------------------------------------
void DebugWindow::markEndTime()
{
    if (m_ShowPerformanceStatistics) {
        m_TimeMarkEnd = std::chrono::steady_clock::now();

        std::chrono::steady_clock::duration startToEndMarkDuration = m_TimeMarkEnd - m_TimeMarkStart;
        double startToEndMarkTimeMs = startToEndMarkDuration.count() / 1000000.0;

        std::chrono::steady_clock::duration drawDuration = m_TimeDrawEnd - m_TimeDrawStart;
        double drawTimeMs = drawDuration.count() / 1000000.0;
        
        m_StartToEndTimings.erase(m_StartToEndTimings.begin());
        m_StartToEndTimings.push_back(startToEndMarkTimeMs);

        m_StartToEndMinusDrawTimings.erase(m_StartToEndMinusDrawTimings.begin());
        m_StartToEndMinusDrawTimings.push_back(startToEndMarkTimeMs - drawTimeMs);
    }
    else {
        std::cout << "DebugWindow::markEndTime() Performance statistics aren't currently enabled." << std::endl;
    }
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
// loadBackgroundTexture()
//---------------------------------------------------------
void DebugWindow::loadBackgroundTexture()
{
    int width, height, channels;
    unsigned char* image = stbi_load_from_memory(logo_png, sizeof(logo_png), &width, &height, &channels, 0);
    if (image != nullptr) {
        GLuint channelInputType = GL_RGB;
        if (channels == 4) {
            channelInputType = GL_RGBA;
        }

        glGenTextures(1, &m_BackgroundTextureHandle);
        glBindTexture(GL_TEXTURE_2D, m_BackgroundTextureHandle);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, channelInputType, GL_UNSIGNED_BYTE, image);
        //glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
    }
}