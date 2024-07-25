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

#include <vector>
#include <set>
#include <string>
#include <functional>
#include <memory>

class DebugWindow
{
    /*
        For each type of Imgui input we want to be able to register, we create a struct of all
        the fields that the input type needs. Then we can build a struct from the corresponding add()
        call and save it back in a vector. Then when draw() is called, we can iterate through
        all registered values.

        TODO:: Move these definitions to their own file
    */

    class ImguiField {
    public:
        virtual void draw() {};
    };

    class SliderFloat : public ImguiField {
    public:
        SliderFloat(std::string _label, float& _f, float _lowerBound, float _upperBound)
            : label(_label), f(_f), lowerBound(_lowerBound), upperBound(_upperBound) {};

        void draw() final {
            ImGui::SliderFloat(label.c_str(), &f, lowerBound, upperBound);
        };
    private:
        std::string label;
        float& f;
        float lowerBound;
        float upperBound;
    };

    class InputText : public ImguiField {
    public:
        InputText(std::string _label, char* _buf, size_t _bufSize)
            : label(_label), buf(_buf), bufSize(_bufSize) {};

        void draw() final {
            ImGui::InputText(label.c_str(), buf, bufSize);
        }
    private:
        std::string label;
        char* buf;
        size_t bufSize;
    };

    // TODO:: Can I enable more than <void(void)> through templating? Is there a reason to?
    class Button : public ImguiField {
    public:
        Button(std::string _label, std::function<void(void)> _callback)
            : label(_label), callback(_callback) {};

        void draw() final {
            if (ImGui::Button(label.c_str()))
                callback();
        }

    private:
        std::string label;
        std::function<void(void)> callback;
    };

    // TODO:: Template this to arithmetic types, add more params from PlotLine
    class PlotLine : public ImguiField {
    public:
        PlotLine(std::string _label, std::vector<float>& _data)
            : label(_label), data(_data) {};

        void draw() final {
            if (ImPlot::BeginPlot(label.c_str())) {
                ImPlot::PlotLine("My Line Plot", data.data(), data.size());
                ImPlot::EndPlot();
            }
        }
    private:
        std::string label;
        std::vector<float>& data;
    };

public:
    DebugWindow();
    ~DebugWindow();

    bool init();
    void cleanup();
    void draw();
    void closeWindow()         { m_Open = false; }
    bool isWindowOpen() const  { return m_Open; }

    void addSliderFloat(const char* label, float& f, float lowerBound, float upperBound);
    void addInputText(const char* label, char* buf, size_t bufSize);
    void addButton(const char* label, std::function<void(void)> callback);
    void addPlotLine(const char* label, std::vector<float>& data);

    void scaleUI(float scale_factor);

private:
    // This is a registery of label names used because Imgui uses labels to decide
    // what part of the gui you're interacting with. If two components have the same label,
    // then it will register input on both when you interact with either. registerAndGetLabel()
    // automatically deconflicts names.
    std::string registerAndGetLabel(std::string label);

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
    uint32_t         m_Width                        { 1280 };
    uint32_t         m_Height                       { 720 };
    // Imgui References
    ImGuiIO*         m_ImguiIo                      { nullptr };
    ImGuiStyle*      m_ImguiStyle                   { nullptr };
    // Class state
    std::set<std::string> m_RegisteredLabels;
    std::vector<std::unique_ptr<ImguiField>> m_RegisteredFields;
    bool m_Open                                     { false };
};

#endif