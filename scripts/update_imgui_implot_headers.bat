REM Imgui

copy ..\extern\imgui\imconfig.h                           ..\include\DebugWindow\imconfig.h
copy ..\extern\imgui\imgui.h                              ..\include\DebugWindow\imgui.h
copy ..\extern\imgui\imgui_internal.h                     ..\include\DebugWindow\imgui_internal.h
copy ..\extern\imgui\imstb_rectpack.h                     ..\include\DebugWindow\imstb_rectpack.h
copy ..\extern\imgui\imstb_textedit.h                     ..\include\DebugWindow\imstb_textedit.h
copy ..\extern\imgui\imstb_truetype.h                     ..\include\DebugWindow\imstb_truetype.h

copy ..\extern\imgui\backends\imgui_impl_opengl3.h        ..\include\DebugWindow\imgui_impl_opengl3.h
copy ..\extern\imgui\backends\imgui_impl_opengl3_loader.h ..\include\DebugWindow\imgui_impl_opengl3_loader.h
copy ..\extern\imgui\backends\imgui_impl_win32.h          ..\include\DebugWindow\imgui_impl_win32.h
copy ..\extern\imgui\backends\imgui_impl_glfw.h           ..\include\DebugWindow\imgui_impl_glfw.h

copy ..\extern\imgui\imgui.cpp                            ..\src\DebugWindow\imgui.cpp
copy ..\extern\imgui\imgui_draw.cpp                       ..\src\DebugWindow\imgui_draw.cpp
copy ..\extern\imgui\imgui_tables.cpp                     ..\src\DebugWindow\imgui_tables.cpp
copy ..\extern\imgui\imgui_widgets.cpp                    ..\src\DebugWindow\imgui_widgets.cpp

copy ..\extern\imgui\backends\imgui_impl_opengl3.cpp      ..\src\DebugWindow\imgui_impl_opengl3.cpp
copy ..\extern\imgui\backends\imgui_impl_win32.cpp        ..\src\DebugWindow\imgui_impl_win32.cpp
copy ..\extern\imgui\backends\imgui_impl_glfw.cpp         ..\src\DebugWindow\imgui_impl_glfw.cpp

REM Implot

copy ..\extern\implot\implot.h                            ..\include\DebugWindow\implot.h
copy ..\extern\implot\implot_internal.h                   ..\include\DebugWindow\implot_internal.h

copy ..\extern\implot\implot.cpp                          ..\src\DebugWindow\implot.cpp
copy ..\extern\implot\implot_items.cpp                    ..\src\DebugWindow\implot_items.cpp