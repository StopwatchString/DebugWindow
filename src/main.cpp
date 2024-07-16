#include "DebugWindow.h"

#include <iostream>

int main(int, char**)
{
    DebugWindow debugWindow;
    debugWindow.init();
    float f = 0.0f;
    debugWindow.addSliderFloat("test", f, -10.0f, 10.0f);

    debugWindow.addButton("the button", []() {std::cout << "pressed" << '\n'; });
    
    float f2 = 1.0f;
    debugWindow.addSliderFloat("test2", f2, -20.0f, 20.0f);

    bool done = false;
    while (!done)
    {
        debugWindow.draw();
        std::cout << f << '\n';

        done = debugWindow.hasExited();
    }

    std::cout << "test" << std::endl;

    return 0;
}