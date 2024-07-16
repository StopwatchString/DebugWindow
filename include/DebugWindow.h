#ifndef DEBUG_WINDOW_H
#define DEBUG_WINDOW_H

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
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

private:
    /*
        For each type of Imgui input we want to be able to register, we create a struct of all
        the fields that the input type needs. Then we can build a struct from the corresponding add()
        call and save it back in a vector. Then when draw() is called, we can iterate through
        all registered values.
    */

    struct SliderFloat {
        std::string label;
        float& f;
        float lowerBound;
        float upperBound;

        SliderFloat(std::string _label, float& _f, float _lowerBound, float _upperBound)
            : label(_label), f(_f), lowerBound(_lowerBound), upperBound(_upperBound) {};
    };
    std::vector<SliderFloat> registeredSliderFloats;

    struct InputText {
        std::string label;
        char* buf;
        size_t bufSize;

        InputText(std::string _label, char* _buf, size_t _bufSize)
            : label(_label), buf(_buf), bufSize(_bufSize) {};
    };
    std::vector<InputText> registeredInputTexts;

    // TODO:: Can I enable more than <void(void)> through templating? Is there a reason to?
    struct Button {
        std::string label;
        std::function<void(void)> callback;

        Button(std::string _label, std::function<void(void)> _callback)
            : label(_label), callback(_callback) {};
    };
    std::vector<Button> registeredButtons;

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
    ImGuiIO& io;
};

#endif