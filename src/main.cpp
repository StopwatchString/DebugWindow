#include "DebugWindow.h"

#include <iostream>

int mainImpl()
{
    DebugWindow debugWindow;
    if (!debugWindow.init()) {
        std::cout << "Error initializing DebugWindow" << std::endl;
    }

    float f = 0.0f;
    debugWindow.addSliderFloat("Float Input", f, -1.0f, 1.0f);

    debugWindow.addButton("Button", []() { std::cout << "Button Pressed" << '\n'; });
    
    float f2 = 1.0f;
    debugWindow.addSliderFloat("Float 2", f2, 0.0f, 1.0f);

    std::vector<float> data;
    for (int i = 0; i < 1000; i++) {
        data.push_back(0);
    }
    debugWindow.addPlotLine("Plot Line", data);

    while (debugWindow.isWindowOpen())
    {
        data.erase(data.begin());
        data.push_back(f2);

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