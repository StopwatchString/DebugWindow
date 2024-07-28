#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "implot.h"

#include "winapiHelpers.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>

/*
    DebugWindow is meant to be a drop-in friendly class for random codebases with any kind of
    frequently called loop. It encapsulates an Imgui instance and hides the cumbersome
    implementation of managing a Win32 instance and deconflicting with any pre-existing OpenGL
    instance.

    Raw Win32 window management and C++ 14 adherence is to provide a higher chance of out of the box
    compatability with outside codebases.

    TODO: More fields from Imgui and Implot
    TODO: Self-managed debug information, like automatically created timing graph based 
          on how often draw() is called.
*/

class DebugWindow
{
    struct ImguiField
    {
        std::string label               { "" };
        bool visible                    { true };
        std::function<void()> drawable  {};
    };

public:
    DebugWindow(uint32_t width = 1280, uint32_t height = 720);
    ~DebugWindow();

    void draw();
    bool isWindowOpen() const  { return m_Open; }

    void addSliderFloat(std::string label, float& f, float lowerBound, float upperBound);
    void addInputText(std::string label, char* buf, size_t bufSize);
    void addButton(std::string label, std::function<void(void)> callback);
    void addInternalPlot(std::string label, uint32_t pointCount = 1000);
    void addExternalPlot(std::string label, std::vector<float>& data);

    void pushToInternalPlot(std::string label, float f);

    // Imgui uses labels to decide what part of the gui you're interacting with. 
    // If two components have the same label, then it will register input on both 
    // when you interact with either. registerLabel() tracks what labels have 
    // been used and transforms the given label into a deconflicted version.
    void registerLabel(std::string& label);
private:


    void init();
    void cleanup();
    void closeWindow()         { m_Open = false; }

    void pushOpenGLState();
    void popOpenGLState();

    static bool     m_PlatformBackendsInit;
    // OpenGL State Management
    HGLRC            m_ReturnOpenGLContext          {};
    HDC              m_ReturnOpenGLDeviceContext    {};
    // Win32 Window Management
    WNDCLASSEXW      m_WindowClass                  {};
    HWND             m_WindowHandle                 {};
    HGLRC            m_HandleRenderContext          {};
    WGL_WindowData   m_MainWindow                   {};
    uint32_t         m_Width                        {};
    uint32_t         m_Height                       {};
    // Imgui References
    ImGuiIO*         m_ImguiIo                      { nullptr };
    ImGuiStyle*      m_ImguiStyle                   { nullptr };
    // Class state
    std::set<std::string> m_RegisteredLabels;
    std::list<ImguiField> m_Drawables;
    std::unordered_map<std::string, std::vector<float>> m_InternalPlotData;
    bool m_Open                                     { false };
};

#endif