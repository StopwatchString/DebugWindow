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

#include <functional>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <list>
#include <array>
#include <chrono>


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
    const static uint32_t MAX_STRING_INPUT_SIZE = 100;

    struct Drawable
    {
        std::string label               { "" };
        bool visible                    { true };
        std::function<void()> draw;
    };

public:
    DebugWindow(uint32_t width = 1280, uint32_t height = 720);
    ~DebugWindow();

    void draw();
    bool isWindowOpen() const  { return m_Open; }

    void addSliderFloat(std::string label, float& f, float lowerBound, float upperBound);
    void addInputText(std::string label, std::string& input);
    void addButton(std::string label, std::function<void(void)> callback);
    void addInternalPlot(std::string label, uint32_t sampleSize = 1000);
    void addExternalPlot(std::string label, std::vector<float>& data);
    void addSameLine();
    void addSpacing(uint32_t count = 1);

    void pushToInternalPlot(std::string label, float f);
    void setVisibility(std::string label, bool visible);

    void enableInternalPerformanceStatistics();
    void markStartTime();
    void markEndTime();

private:


    void init();
    void cleanup();
    void closeWindow()         { m_Open = false; }

    void pushOpenGLState();
    void popOpenGLState();
    int  getNextId()           { return ++m_ImGuiIdCount; }

    void loadBackgroundTexture();

    static bool         m_PlatformBackendsInit;
    // OpenGL State Management
    HGLRC               m_ReturnOpenGLContext          {};
    HDC                 m_ReturnOpenGLDeviceContext    {};
    // Win32 Window Management
    WNDCLASSEXW         m_WindowClass                  {};
    HWND                m_WindowHandle                 {};
    WGL_WindowData      m_MainWindow                   {};
    uint32_t            m_Width                        {};
    uint32_t            m_Height                       {};
    // Imgui References
    ImGuiIO*            m_ImguiIo                      { nullptr };
    ImGuiStyle*         m_ImguiStyle                   { nullptr };
    // Perf Tracking
    bool                m_ShowPerformanceStatistics    { false };
    std::string         m_PerformanceStatisticsID      { "m_PerformanceStatisticsID" };
    double              m_LastFrameDrawTimeMs          { 0.0 };
    std::vector<double> m_StartToEndTimings;
    std::vector<double> m_EndToStartTimings;
    std::vector<double> m_StartToEndMinusDrawTimings;
    std::chrono::steady_clock::time_point m_TimeDrawStart   { std::chrono::steady_clock::now() };
    std::chrono::steady_clock::time_point m_TimeDrawEnd     { std::chrono::steady_clock::now() };
    std::chrono::steady_clock::time_point m_TimeMarkStart   { std::chrono::steady_clock::now() };
    std::chrono::steady_clock::time_point m_TimeMarkEnd     { std::chrono::steady_clock::now() };

    uint32_t            m_ImGuiIdCount                 { 0 };
    GLuint              m_BackgroundTextureHandle      { 0 };
    bool                m_Open                         { false };
    std::list<Drawable> m_Drawables;
    std::unordered_map<std::string, std::vector<float>> m_InternalPlotData;
    std::unordered_map<std::string, std::array<char, MAX_STRING_INPUT_SIZE>> m_InputStringData;
};

#endif