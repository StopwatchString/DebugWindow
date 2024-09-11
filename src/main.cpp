#include "DebugWindowGLFW.h"
#include "DebugWindowWin32.h"

#include <windows.h>
#include <iostream>

int mainImpl()
{
    std::string floatLabel = "Float";
    std::string externallyManagedPlotLabel = "External Plot";
    std::string internallyManagedPlotLabel = "Internal Plot";

#ifdef DEBUGWINDOW_WIN32
    DebugWindowWin32 debugWindow;
#endif
#ifdef DEBUGWINDOW_GLFW
    DebugWindowGLFW debugWindow;
#endif

    float f1 = 1.0f;
    float f2 = 1.0f;
    debugWindow.addSliderFloat(floatLabel, f1, 0.0f, 1.0f);
    debugWindow.addSliderFloat(floatLabel, f2, 0.0f, 1.0f);
    
    debugWindow.addInternalPlot(internallyManagedPlotLabel);

    debugWindow.enableInternalPerformanceStatistics();

    while (debugWindow.isWindowOpen())
    {
        debugWindow.markStartTime();

        debugWindow.pushToInternalPlot(internallyManagedPlotLabel, f2);
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