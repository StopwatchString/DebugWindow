# DebugWindow

    DebugWindow is meant to be a drop-in friendly class for random codebases with any kind of
    frequently called loop. It encapsulates an Imgui instance and hides the cumbersome
    implementation of managing a Win32 instance and deconflicting with any pre-existing OpenGL
    instance.

    Raw Win32 window management and C++ 14 adherence is to provide a higher chance of out of the box
    compatability with outside codebases.

    TODO: More fields from Imgui and Implot
    TODO: Self-managed debug information, like automatically created timing graph based 
          on how often draw() is called.