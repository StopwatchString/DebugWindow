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

    struct Drawable
    {
        std::string label               { "" };
        bool visible                    { true };
        std::function<void()> draw;
    };

public:
    DebugWindow() {};
    ~DebugWindow() {};

    virtual void draw();
    bool isWindowOpen() const  { return m_Open; }

    virtual void addSliderFloat(std::string label, float& f, float lowerBound, float upperBound) final;
    virtual void addInputText(std::string label, std::string& input) final;
    virtual void addButton(std::string label, std::function<void(void)> callback) final;
    virtual void addInternalPlot(std::string label, uint32_t sampleSize = 1000) final;
    virtual void addExternalPlot(std::string label, std::vector<float>& data) final;
    virtual void addSameLine() final;
    virtual void addSpacing(uint32_t count = 1) final;

    virtual void pushToInternalPlot(std::string label, float f) final;
    virtual void setVisibility(std::string label, bool visible) final;

    virtual void enableInternalPerformanceStatistics() final;
    virtual void markStartTime() final;
    virtual void markEndTime() final;

    virtual void closeWindow() final;
protected:
    virtual void init() = 0;
    virtual void cleanup() = 0;
    virtual void drawImpl() = 0;
    virtual void toggleVsync() = 0;
    bool m_VsyncEnabled{ false };

    virtual void pushOpenGLState() = 0;
    virtual void popOpenGLState() = 0;
    
    virtual void drawWindow() final;

    static bool m_PlatformBackendsInit;
    bool m_Open { false };
    uint32_t            m_Width{};
    uint32_t            m_Height{};

    const char* OS_WINDOW_NAME = "Imgui Window";
    const char* IMGUI_PANEL_NAME = "Debug Panel";
    const uint32_t DEFAULT_WINDOW_WIDTH = 1280;
    const uint32_t DEFAULT_WINDOW_HEIGHT = 720;

private:

    int  getNextId()           { return ++m_ImGuiIdCount; }

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
    std::list<Drawable> m_Drawables;
    std::unordered_map<std::string, std::vector<float>> m_InternalPlotData;
    std::unordered_map<std::string, std::array<char, MAX_STRING_INPUT_SIZE>> m_InputStringData;
};

#endif