# DebugWindow
DebugWindow is meant to be a quick-use wrapper around the popular DearImgui (+Implot) library. Features include:

- Self-managed OpenGL context designed to be transparent to the calling program.
- Add fields to the window a single time, then just call draw() once per frame wherever your loop is.
- Add plots both for data you track (external vectors) or create an 'Internal Plot.' Internal Plots will keep track of your data as you push to it, letting you track how values change over time without needing to set up a member variable or static storage yourself.
- A drop-in performance statistic graph. Just enable, call markStartTime() and markEndTime() around the blocks you want to time, and get a graph of the timings.
- Wrapper for InputString() to reflect into a std::string
- More planned!

Currently there are two different backend implementations, aiming at different use cases-

Raw Win32- In my experience, many codebases are fragile and you'll break their build adding more than the absolute bare minimum. While DearImgui carefully deconflicts itself by aliasing the functions most likely to run into conflicts, the same cannot be said for common window manager libraries like GLFW. It has been my experience that directly relying on the Win32 api for window management has made integration significantly simpler. The only drawback is that Imgui has worse support for raw Win32 than other backends (even actively advising against it), and I'm not Windows API wizard. Odd behavior and the rare crash may occur on this backend.

GLFW- The obvious, simple, multiplatform window management library. This is the goto version if you aren't concerned about conflicts, like if you're relying on this window to be a standalone tool.
