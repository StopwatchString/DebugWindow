#include "DebugWindowGLFW.h"
#include "DebugWindowWin32.h"

#include <windows.h>
#include <iostream>

int mainImpl()
{
    std::string floatField = "Float";
    std::string externalPlot = "External Plot";
    std::string internalPlot = "Internal Plot";
    std::string internalPlotToggle = "Toggle Internal Plot";
    std::string closeWindow = "Close Window";

#ifdef DEBUGWINDOW_WIN32
    DebugWindowWin32 debugWindowImpl;
#endif
#ifdef DEBUGWINDOW_GLFW
    DebugWindowGLFW debugWindowImpl;
#endif

    DebugWindow& debugWindow = debugWindowImpl;

    float f1 = 1.0f;
    float f2 = 1.0f;
    debugWindow.addSliderFloat(floatField, f1, 0.0f, 1.0f);
    debugWindow.addSliderFloat(floatField, f2, 0.0f, 1.0f);
    
    debugWindow.addInternalPlot(internalPlot);

    debugWindow.addButton(internalPlotToggle, [&]() {
        static bool visible = true;
        visible = !visible;
        debugWindow.setVisibility(internalPlot, visible);
    });

    debugWindow.enableInternalPerformanceStatistics();

    while (debugWindow.isWindowOpen())
    {
        debugWindow.markStartTime();

        debugWindow.pushToInternalPlot(internalPlot, f2);
        debugWindow.draw();

        debugWindow.markEndTime();
    }

    return EXIT_SUCCESS;
}

#ifdef NDEBUG
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    return mainImpl();
}
#else
int main(int, char**)
{
    return mainImpl();
}
#endif