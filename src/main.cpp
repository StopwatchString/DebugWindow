#include "DebugWindow.h"

#include <iostream>

int mainImpl()
{
    DebugWindow debugWindow(2000, 1000);
    float f2 = 1.0f;
    debugWindow.addSliderFloat("Float 2", f2, 0.0f, 1.0f);

    std::vector<float> externalPlotData;
    for (int i = 0; i < 1000; i++) {
        externalPlotData.push_back(0);
    }
    debugWindow.addExternalPlot("External Plot", externalPlotData);

    debugWindow.addInternalPlot("Internal Plot");

    bool running = true;
    debugWindow.addButton("Close Window", [&]() { running = false; });

    while (running)
    {
        externalPlotData.erase(externalPlotData.begin());
        externalPlotData.emplace_back(f2);
        
        debugWindow.pushToInternalPlot("Internal Plot", f2);

        debugWindow.draw();
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