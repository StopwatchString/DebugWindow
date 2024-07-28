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
    float f2 = 1.0f;
    debugWindow.addSliderFloat(floatField, f2, 0.0f, 1.0f);

    std::vector<float> externalPlotData;
    for (int i = 0; i < 1000; i++) {
        externalPlotData.push_back(0);
    }
    debugWindow.addExternalPlot(externalPlot, externalPlotData);

    debugWindow.addInternalPlot(internalPlot);

    debugWindow.addButton(internalPlotToggle, [&]() {
        static bool visible = true;
        visible = !visible;
        debugWindow.setVisibility(internalPlot, visible);
    });

    std::string input;
    debugWindow.addInputText("Input String", input);
    std::string input2;
    debugWindow.addInputText("Input a", input2);

    bool running = true;
    debugWindow.addButton(closeWindow, [&]() { running = false; });

    while (running)
    {
        externalPlotData.erase(externalPlotData.begin());
        externalPlotData.emplace_back(f2);

        debugWindow.pushToInternalPlot(internalPlot, f2);

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