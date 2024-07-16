#include "DebugWindow.h"

#include <iostream>

int main(int, char**)
{
    DebugWindow debugWindow;
    debugWindow.init();
    float f = 0.0f;
    debugWindow.addSliderFloat("Float", f, -1.0f, 1.0f);

    debugWindow.addButton("Button", []() {std::cout << "pressed" << '\n'; });
    
    float f2 = 1.0f;
    debugWindow.addSliderFloat("Float 2", f2, 0.0f, 1.0f);

    std::vector<float> data;

    debugWindow.addPlotLine("Plot Line", data);

    for (int i = 0; i < 1000; i++) {
        data.push_back(0);
    }

    bool done = false;
    while (!done)
    {
        data.erase(data.begin());
        data.push_back(f2);
        debugWindow.draw();
        std::cout << f << '\n';

        done = debugWindow.hasExited();
    }

    std::cout << "test" << std::endl;

    return 0;
}