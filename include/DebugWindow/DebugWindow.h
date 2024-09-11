/*
 Abstract Class: Debug Window
 Author:         Mason Speck
 
  Encapsulation of the DearImgui (+Implot) library to spawn a 
  standalone window from any application context. Implementations
  manage their own OpenGL context so that they don't conflict
  with the parent application. Designed primarily as a tool to
  provide a low friction drop-in debugging gui to applications
  which have otherwise not been written to have the capability.

  Interface allows for 'registering' fields as Drawables with the typical inputs
  DearImgui requires. Drawables include function objects that 
  encapsulate all of the DearImgui boilerplate for drawing fields
  and store references back to any fields they modify. Then draw()
  iterates through all of the Drawables. This allows the user 
  to sequester away the bulk of the imgui boilerplate in an init
  function somewhere, only needing to call draw() within their loop.
 */

#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include "imgui.h"
#include "implot.h"

#include <functional>
#include <vector>
#include <chrono>
#include <list>
#include <unordered_map>
#include <array>
#include <string>

class DebugWindow
{
    const static uint32_t MAX_STRING_INPUT_SIZE = 100;
    const static uint32_t DEFAULT_INTERNAL_PLOT_SAMPLE_SIZE = 1000;

    struct Drawable
    {
        std::string label          { "" };
        bool visible               { true }; // TODO:: Mech up visibility toggle
        std::function<void()> draw {};
    };

public:
    DebugWindow()  = default;
    ~DebugWindow() = default;

    void draw();
    bool isWindowOpen() const  { return m_Open; }
    void closeWindow();

    void addSliderFloat(std::string label, float& f, float lowerBound, float upperBound, bool showResetButton = false);
    void addSliderInt(std::string label, int& i, int lowerBound, int upperBound, bool showResetButton = false);
    void addInputText(std::string label, std::string& input);
    void addButton(std::string label, std::function<void(void)> callback);
    void addInternalPlot(std::string label, uint32_t sampleSize = DEFAULT_INTERNAL_PLOT_SAMPLE_SIZE);
    void pushToInternalPlot(std::string label, float f);
    void addExternalPlot(std::string label, std::vector<float>& data);
    void addSameLine();
    void addSpacing(uint32_t count = 1);

    void enableInternalPerformanceStatistics();
    void markStartTime();
    void markEndTime();

protected:
    // Implementation classes should implement all of these functions
    virtual void init()                       = 0;
    virtual void cleanup()                    = 0;
    virtual void implementationDrawWrapper()  = 0;
    virtual void toggleVsync()                = 0;
    virtual void pushOpenGLState()            = 0;
    virtual void popOpenGLState()             = 0;

    // Implementation classes should call this inbetween impl specific code in implementationDrawWrapper()
    void drawImguiElements();

    // Used to ensure Imgui backends are only init once per process
    static bool    m_PlatformBackendsInit;

    bool           m_Open                 { false };
    bool           m_VsyncEnabled         { false };
    uint32_t       m_Width                {};
    uint32_t       m_Height               {};

    const char*    OS_WINDOW_NAME         { "Imgui Window" };
    const char*    IMGUI_PANEL_NAME       { "Debug Panel" };
    const uint32_t DEFAULT_WINDOW_WIDTH   { 1280 };
    const uint32_t DEFAULT_WINDOW_HEIGHT  { 720 };

private:
    int getNextId() { return ++m_ImGuiIdCount; }

    // Perf State
    bool                m_ShowPerformanceStatistics         { false };
    std::string         m_PerformanceStatisticsID           { "m_PerformanceStatisticsID" };
    double              m_LastFrameDrawTimeMs               { 0.0 };
    std::vector<double> m_StartToEndTimings;
    std::vector<double> m_EndToStartTimings;
    std::vector<double> m_StartToEndMinusDrawTimings;
    std::chrono::steady_clock::time_point m_TimeDrawStart   { std::chrono::steady_clock::now() };
    std::chrono::steady_clock::time_point m_TimeDrawEnd     { std::chrono::steady_clock::now() };
    std::chrono::steady_clock::time_point m_TimeMarkStart   { std::chrono::steady_clock::now() };
    std::chrono::steady_clock::time_point m_TimeMarkEnd     { std::chrono::steady_clock::now() };

    // Drawables State
    uint32_t m_ImGuiIdCount { 0 };
    std::list<Drawable> m_Drawables;
    std::unordered_map<std::string, std::vector<float>> m_InternalPlotData;
    std::unordered_map<std::string, std::array<char, MAX_STRING_INPUT_SIZE>> m_InputStringData;
};

#endif