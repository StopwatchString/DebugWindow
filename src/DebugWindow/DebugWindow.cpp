#include "DebugWindow.h"

#include <iostream>

bool DebugWindow::m_PlatformBackendsInit = false;

//---------------------------------------------------------
// draw() - Wraps implementation draw to allow for
//          meta work we don't want to duplicate in
//          implementations.
//---------------------------------------------------------
void DebugWindow::draw()
{
    if (m_Open) {
        m_TimeDrawStart = std::chrono::steady_clock::now();

        implementationDrawWrapper();

        m_TimeDrawEnd = std::chrono::steady_clock::now();

        if (!m_Open) {
            cleanup();
        }
    }
    else {
        std::cout << "DebugWindow::draw() Window isn't open." << '\n';
    }
}

//---------------------------------------------------------
// drawWindow() - Implementations call this within
//                implementationDrawWrapper() to actually
//                handle traversing the Drawables list
//---------------------------------------------------------
void DebugWindow::drawImguiElements()
{
    ImGui::NewFrame();
    ImGui::Begin(IMGUI_PANEL_NAME, &m_Open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::MenuItem("Toggle Vsync", nullptr, nullptr)) {
                m_VsyncEnabled = !m_VsyncEnabled;
                toggleVsync();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    for (const Drawable drawable : m_Drawables) {
        if (drawable.visible) {
            drawable.draw();
        }
    }

    ImGui::End();
    ImGui::Render();
}

//---------------------------------------------------------
// addSliderFloat()
//---------------------------------------------------------
void DebugWindow::addSliderFloat(std::string label, float& f, float lowerBound, float upperBound, bool showResetButton)
{
    uint32_t id = getNextId();
    if (showResetButton) {
        float resetValue = f;
        Drawable resetButtonField;
        resetButtonField.label = label + "##" + std::to_string(id) + "##ResetButton";
        resetButtonField.draw = [label, id, &f, lowerBound, upperBound, resetValue] {
            ImGui::PushID(id);

            if (ImGui::Button("Reset")) {
                f = resetValue;
            }

            ImGui::PopID();
        };

        m_Drawables.push_back(resetButtonField);

        addSameLine();
    }

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
// addSliderInt()
//---------------------------------------------------------
void DebugWindow::addSliderInt(std::string label, int& i, int lowerBound, int upperBound, bool showResetButton)
{
    uint32_t id = getNextId();
    if (showResetButton) {
        int resetValue = i;
        Drawable resetButtonField;
        resetButtonField.label = label + "##" + std::to_string(id) + "##ResetButton";
        resetButtonField.draw = [label, id, &i, lowerBound, upperBound, resetValue] {
            ImGui::PushID(id);

            if (ImGui::Button("Reset")) {
                i = resetValue;
            }

            ImGui::PopID();
        };

        m_Drawables.push_back(resetButtonField);

        addSameLine();
    }

    Drawable field;
    field.label = label + "##" + std::to_string(id);
    field.draw = [label, id, &i, lowerBound, upperBound]() {
        ImGui::PushID(id);

        ImGui::SliderInt(label.c_str(), &i, lowerBound, upperBound);

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
// addInternalPlot() - An 'internal' plot is one that plots
//                     a vector stored within DebugWindow.
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
// pushToInternalPlot() - Push values into an internal plot.
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
// addExternalPlot() - An 'external' plot is a reference to
//                     a vector not managed by DebugWindow.
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
// addSameLine() - Adds a 'SameLine' call to DearImgui, causing
//                 the next element to occupy the same line
//                 as the one preceding it.
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
// addSpacing() - Adds a 'Spacing' call to DearImgui,
//                which adds vertical padding between the
//                preceding element and the next element.
//                Effect stacks, so an amount count can be
//                specified.
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
// enableInternalPerformanceStatistics() -
//      InternalPerformanceStatistics refers to an
//      internally kept vector of timings that are
//      updated by calling markStartTime() and markEndTime().
//      
//      Meant to be a quick access way to evaluate major performance
//      issues, but should not be relied on for proper benchmarking.
// 
//      DebugWindow by design creates an additional OpenGL context
//      and switches to it every frame to update the window.
//      This really pollutes fine-grain benchmarking, and that
//      doesn't even include the time it takes to draw the window itself.
//      (avg is ~2ms in my experience)
//      The timings emitted include stats that subtract out the time
//      it takes for DebugWindow draw() anyway, which includes the context switch. 
// 
//      I find this valuable as an early warning siren for something
//      going wrong, but to properly evaluate performance DebugWindow
//      must be disabled entirely.
//---------------------------------------------------------
void DebugWindow::enableInternalPerformanceStatistics()
{
    // The drawable related to internal perf statistics can only be added a single time
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
// closeWindow()
//---------------------------------------------------------
void DebugWindow::closeWindow()
{
    m_Open = false;
    cleanup();
}