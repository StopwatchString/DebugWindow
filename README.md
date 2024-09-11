# DebugWindow

![image](https://github.com/user-attachments/assets/504082c2-af77-4bb5-b012-8ff9a5b92ac4)

## Introduction
DebugWindow is meant to be a quick-use wrapper around the popular DearImgui (+Implot) library. It has 3 main priorities:

- Create a standalone window using DearImgui managing its own OpenGL context to minimize conflicts
- Provide a simplified interface into DearImgui/Implot that aims to minimize code pollution
- Be easily included into an existing project that already has some kind of realtime loop

Once included in your project, typical usage looks like this:

```cpp

// Declare early in scope or as a class member somewhere (choosing an implementation type)
DebugWindowWin32 debugWindow;

.....

// During your program's initialization, add the fields you want drawn

debugWindow.addSliderFloat("float1", f1, 0.0f, 1.0f);
debugWindow.addSliderFloat("float2", f2, 0.0f, 1.0f);
debugWindow.addInternalPlot("Internal Plot");
debugWindow.addExternalPlot("External Plot", sinVals);
debugWindow.addButton(buttonLabel, []() {
    std::cout << "Button pressed" << std::endl;
});

.....

// Later, within your realtime loop, just draw

debugWindow.draw();
```

You can also add fields dynamically later, though adding to the draw list is currently not thread safe.

In the case that you are using an internally tracked plot of data, then you just need to add this call in your loop whereever the data you're tracking is located:

```cpp
debugWindow.pushToInternalPlot("Internal Plot", val);
```

### Internal Performance Statistics

![image](https://github.com/user-attachments/assets/cd95127b-4db4-4373-ac89-ae175b40586b)

DebugWindow also supports automatically creating a plot of timing statistics.

```cpp
// During initialization
debugWindow.enableInteralPerformanceStatistics();

// At the start and end of your realtime loop (or just the section that you're timing)

debugWindow.markStartTime();
.....
debugWindow.markEndTime();
```

This will create a performance graph within the debug window using the times associated with the start and end you're measuring.

**Note:** DebugWindow is a poor measure for the performance characteristics of your program, somewhat by design. It manages its own OpenGL context and switches to it every frame, polluting any microbenchmarks you might attempt. In addition, the window itself takes ~2ms to draw. This automatic performance graphing provided then is mostly useful as a warning sign for major issues. True benchmarking should only occur with the DebugWindow disabled entirely.

### Different Backend Choices

Currently there are two different choices for Window and OpenGL context management backends, Win32 and GLFW. 

Typically GLFW is always the preferred choice, as it is crossplatform, battletested, and just _isn't_ the raw Win32 API. The reason both are offered is twofold- include conflicts, and the extra code you must link against. One of the core goals stated above is that this tool should be easy to integrate and try to have zero conflicts with any project its dropped into. In my experience, GLFW can be hard to include if the project you're integrating with is already using an older version. For this reason, the Win32 API backend version was created.

In practice, the Win32 API variant is slightly buggier (but also slightly faster !). DearImgui itself in the example code for Win32 API backend recommends against using it, suggesting that it's not well supported. This has been borne out in my usage of it through odd behavior and the rare crash.

For practical purposes, the GLFW variant is slightly more stable but much heavier to include. I tend to just use the Win32 version, since I only need the headers and source files.

## Usage

Platform for example code is assumed to be Windows.

### To use provided solution
- Clone the repo
- To build, either execute ```cmake -S ./ -B ./build``` at the root of the project or navigate to /scripts and run gen_solution.bat (which does the same thing)
- Open the solution generated in /build. You can toggle between startup projects on the right to run either the GLFW or Win32 version. They both use the same main.cpp.

### To pull into your own project
- Copy the contents of /include and /src to your project's equivalent directories
- Add /include/DebugWindow to your include path
- In the case that you use the GLFW version, copy glfw3.lib from /lib and link against it in your main project. Also check that you copied /include/GLFW/ from this project as well

CMakeLists.txt supporting add_subdirectory() will come soon.

## TODO
    -Visibility toggle
    -More generic menu options like Vsync
    -Lots more fields from Imgui and Implot
    -Data exporting from plots
    -Plot configuration options in interface
    -Return optional mutexes from fields like Button that function like callbacks
    -Support for multiple windows at once
