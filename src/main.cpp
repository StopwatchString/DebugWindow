#include "DebugWindowGLFW.h"
#include "DebugWindowWin32.h"

#include <windows.h>
#include <iostream>

int mainImpl()
{
    std::string floatLabel = "Float";
    std::string externallyManagedPlotLabel = "External Plot";
    std::string internallyManagedPlotLabel = "Internal Plot";
    std::string buttonLabel = "Print to Console";

#ifdef DEBUGWINDOW_WIN32
    DebugWindowWin32 debugWindow;
#endif
#ifdef DEBUGWINDOW_GLFW
    DebugWindowGLFW debugWindow;
#endif

    float f1 = 1.0f;
    float f2 = 1.0f;
    
    float time = 0.0f;
    std::vector<float> sinVals;
    for (int i = 0; i < 500; ++i) {
        sinVals.push_back(-1.0f);
        sinVals.push_back(1.0f);
    }

    debugWindow.addSliderFloat(floatLabel, f1, 0.0f, 1.0f);
    debugWindow.addSliderFloat(floatLabel, f2, 0.0f, 1.0f);
    debugWindow.addInternalPlot(internallyManagedPlotLabel);
    debugWindow.addExternalPlot(externallyManagedPlotLabel, sinVals);
    debugWindow.enableInternalPerformanceStatistics();
    debugWindow.addButton(buttonLabel, []() {
        std::cout << "Button pressed" << std::endl;
    });

    while (debugWindow.isWindowOpen())
    {
        debugWindow.markStartTime();

        // Updating interally tracked plot
        debugWindow.pushToInternalPlot(internallyManagedPlotLabel, f2);

        // Updating externally tracked plot
        time += 0.1f;
        sinVals.erase(sinVals.begin());
        sinVals.push_back(sin(time));

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