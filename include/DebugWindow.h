#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "implot.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <GL/GL.h>
#include <tchar.h>

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>

class DebugWindow
{
public:
    DebugWindow();
    ~DebugWindow();

    bool init();
    void cleanup();
    void draw();
    bool hasExited() { return exited; }

    void addSliderFloat(const char* label, float& f, float lowerBound, float upperBound);
    void addInputText(const char* label, char* buf, size_t bufSize);
    void addButton(const char* label, std::function<void(void)> callback);
    void addPlotLine(const char* label, std::vector<float>& data);

private:

    /*
        For each type of Imgui input we want to be able to register, we create a struct of all
        the fields that the input type needs. Then we can build a struct from the corresponding add()
        call and save it back in a vector. Then when draw() is called, we can iterate through
        all registered values.
    */

    class ImguiField {
    public:
        virtual void draw() {};
    };

    std::vector<std::unique_ptr<ImguiField>> registeredFields;

    struct SliderFloat : public ImguiField {
        std::string label;
        float& f;
        float lowerBound;
        float upperBound;

        SliderFloat(std::string _label, float& _f, float _lowerBound, float _upperBound)
            : label(_label), f(_f), lowerBound(_lowerBound), upperBound(_upperBound) {};

        void draw() final {
            ImGui::SliderFloat(label.c_str(), &f, lowerBound, upperBound);
        };
    };

    struct InputText : ImguiField {
        std::string label;
        char* buf;
        size_t bufSize;

        InputText(std::string _label, char* _buf, size_t _bufSize)
            : label(_label), buf(_buf), bufSize(_bufSize) {};

        void draw() final {
            ImGui::InputText(label.c_str(), buf, bufSize);
        }
    };

    // TODO:: Can I enable more than <void(void)> through templating? Is there a reason to?
    struct Button : ImguiField {
        std::string label;
        std::function<void(void)> callback;

        Button(std::string _label, std::function<void(void)> _callback)
            : label(_label), callback(_callback) {};

        void draw() final {
            if (ImGui::Button(label.c_str()))
                callback();
        }
    };

    struct PlotLine : ImguiField {
        std::string label;
        std::vector<float>& data;

        PlotLine(std::string _label, std::vector<float>& _data)
            : label(_label), data(_data) {};

        void draw() final {
            if (ImPlot::BeginPlot(label.c_str())) {
                ImPlot::PlotLine("My Line Plot", data.data(), data.size());
                ImPlot::EndPlot();
            }
        }
    };

    // This is a registery of label names used because Imgui uses labels to decide
    // what part of the gui you're interacting with. If two components have the same label,
    // then it will register input on both when you interact with either. registerAndGetLabel()
    // automatically deconflicts names.
    std::unordered_map<std::string, int> registeredLabels;
    std::string registerAndGetLabel(const char* label);

    // Win32 Window Management
    WNDCLASSEXW wc;
    HWND hwnd;

    bool exited{ false };
    bool initialized         { false };

    // Imgui Members
    ImGuiIO io;
};

#endif