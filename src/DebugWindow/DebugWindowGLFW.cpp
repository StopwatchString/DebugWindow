#include "DebugWindowGLFW.h"

//---------------------------------------------------------
// glfw_error_callback()
//---------------------------------------------------------
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

//---------------------------------------------------------
// Constructor
//---------------------------------------------------------
DebugWindowGLFW::DebugWindowGLFW()
{
    init();
}

//---------------------------------------------------------
// Destructor
//---------------------------------------------------------
DebugWindowGLFW::~DebugWindowGLFW()
{
    if (m_Open) {
        cleanup();
        m_Open = false;
    }
}

//---------------------------------------------------------
// init()
//---------------------------------------------------------
void DebugWindowGLFW::init()
{
    pushOpenGLContext();

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return;
    }

    const char* glsl_version = "#version 460";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    // Create window with graphics context
    m_Window = glfwCreateWindow(1, 1, OS_WINDOW_NAME, nullptr, nullptr);
    if (m_Window == nullptr)
        return;
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(0); // Start with Vsync disabled

    // Setup DearImgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    // Styling DearImgui
    ImGui::StyleColorsDark();
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends, but only once
    if (!m_PlatformBackendsInit) {
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
        m_PlatformBackendsInit = true;
    }

    m_Open = true;

    popOpenGLContext();
}

//---------------------------------------------------------
// cleanup()
//---------------------------------------------------------
void DebugWindowGLFW::cleanup()
{
    pushOpenGLContext();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(m_Window);
    //glfwTerminate(); For now forgo terminating GLFW since we don't know if we should be in charge of terminating

    popOpenGLContext();
}

//---------------------------------------------------------
// drawImpl()
//---------------------------------------------------------
void DebugWindowGLFW::implementationDrawWrapper()
{
    pushOpenGLContext();

    glfwPollEvents();
    if (glfwGetWindowAttrib(m_Window, GLFW_ICONIFIED) != 0)
    {
        ImGui_ImplGlfw_Sleep(10);
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    // Call to DebugWindow class to handle building the Imgui draw list
    drawImguiElements();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // Sometimes the actual DebugWindow is not the primary window, and is updated here as a viewport.
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        // Not sure if this is necessary to toggle Vsync when we are a viewport
        //if (m_VsyncEnabled) {
        //    glfwSwapInterval(1);
        //}
        //else {
        //    glfwSwapInterval(0);
        //}

        glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(m_Window);

    popOpenGLContext();
}

//---------------------------------------------------------
// toggleVsync()
//---------------------------------------------------------
void DebugWindowGLFW::toggleVsync()
{
    if (m_VsyncEnabled) {
        glfwSwapInterval(1);
    }
    else {
        glfwSwapInterval(0);
    }
}

//---------------------------------------------------------
// pushOpenGLState()
//---------------------------------------------------------
void DebugWindowGLFW::pushOpenGLContext()
{
    m_ReturnOpenGLContext = glfwGetCurrentContext();
    glfwMakeContextCurrent(m_Window);
}

//---------------------------------------------------------
// popOpenGLState()
//---------------------------------------------------------
void DebugWindowGLFW::popOpenGLContext()
{
    glfwMakeContextCurrent(m_ReturnOpenGLContext);
}