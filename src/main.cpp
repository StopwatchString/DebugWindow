#include "DebugWindow.h"

#include <iostream>

int main(int, char**)
{
    DebugWindow debugWindow;
    debugWindow.init();
    float f = 0.0f;
    debugWindow.addSliderFloat("test", f, -10.0f, 10.0f);

    bool done = false;
    while (!done)
    {
        debugWindow.draw();
        std::cout << f << '\n';

        done = debugWindow.hasExited();
    }

    return 0;
}