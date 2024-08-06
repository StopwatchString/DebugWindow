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
    window = glfwCreateWindow(1, 1, OS_WINDOW_NAME, nullptr, nullptr);
    if (window == nullptr)
        return;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    m_Open = true;
}

//---------------------------------------------------------
// cleanup()
//---------------------------------------------------------
void DebugWindowGLFW::cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

//---------------------------------------------------------
// drawImpl()
//---------------------------------------------------------
void DebugWindowGLFW::drawImpl()
{

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
    {
        ImGui_ImplGlfw_Sleep(10);
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    drawWindow();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        // Not sure if this is necessary yet
        //if (m_VsyncEnabled) {
        //    glfwSwapInterval(1);
        //}
        //else {
        //    glfwSwapInterval(0);
        //}

        glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(window);
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