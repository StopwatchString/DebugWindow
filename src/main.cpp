#include "DebugWindow.h"

#include <iostream>

int mainImpl()
{
    DebugWindow debugWindow;

    float f = 0.0f;
    debugWindow.addSliderFloat("Float Input", f, -1.0f, 1.0f);

    debugWindow.addButton("Button", []() { std::cout << "Button Pressed" << '\n'; });
    
    float f2 = 1.0f;
    debugWindow.addSliderFloat("Float 2", f2, 0.0f, 1.0f);

    std::vector<float> data;
    for (int i = 0; i < 1000; i++) {
        data.push_back(0);
    }
    debugWindow.addExternalPlot("Plot Line", data);

    debugWindow.addInternalPlot("Plot Internal");

    bool running = true;
    debugWindow.addButton("Close Window", [&]() { running = false; });

    debugWindow.pushToInternalPlot("Plot Internal", f2);
    while (running)
    {
        data.erase(data.begin());
        data.emplace_back(f2);
        
        debugWindow.pushToInternalPlot("Plot Internal", f2);

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