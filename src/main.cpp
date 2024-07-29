#include "DebugWindow.h"

#include <iostream>

int mainImpl()
{
    std::string floatField = "Float";
    std::string externalPlot = "External Plot";
    std::string internalPlot = "Internal Plot";
    std::string internalPlotToggle = "Toggle Internal Plot";
    std::string closeWindow = "Close Window";

    DebugWindow debugWindow(2000, 1000);

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

    bool running = true;
    debugWindow.addButton(closeWindow, [&]() { running = false; });

    // Lets us see the background by scrolling down
    debugWindow.addSpacing(300);

    while (running)
    {
        debugWindow.markStartTime();

        debugWindow.pushToInternalPlot(internalPlot, f2);
        debugWindow.draw();

        debugWindow.markEndTime();
    }

    return 0;
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