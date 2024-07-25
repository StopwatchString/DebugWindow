REM Imgui

copy ..\extern\imgui\imconfig.h ..\include\imgui\imconfig.h
copy ..\extern\imgui\imgui.h ..\include\imgui\imgui.h
copy ..\extern\imgui\imgui_internal.h ..\include\imgui\imgui_internal.h
copy ..\extern\imgui\imstb_rectpack.h ..\include\imgui\imstb_rectpack.h
copy ..\extern\imgui\imstb_textedit.h ..\include\imgui\imstb_textedit.h
copy ..\extern\imgui\imstb_truetype.h ..\include\imgui\imstb_truetype.h

copy ..\extern\imgui\backends\imgui_impl_opengl3.h ..\include\imgui\imgui_impl_opengl3.h
copy ..\extern\imgui\backends\imgui_impl_opengl3_loader.h ..\include\imgui\imgui_impl_opengl3_loader.h
copy ..\extern\imgui\backends\imgui_impl_win32.h ..\include\imgui\imgui_impl_win32.h

copy ..\extern\imgui\imgui.cpp ..\src\imgui\imgui.cpp
copy ..\extern\imgui\imgui_draw.cpp ..\src\imgui\imgui_draw.cpp
copy ..\extern\imgui\imgui_tables.cpp ..\src\imgui\imgui_tables.cpp
copy ..\extern\imgui\imgui_widgets.cpp ..\src\imgui\imgui_widgets.cpp

copy ..\extern\imgui\backends\imgui_impl_opengl3.cpp ..\src\imgui\imgui_impl_opengl3.cpp
copy ..\extern\imgui\backends\imgui_impl_win32.cpp ..\src\imgui\imgui_impl_win32.cpp

REM Implot

copy ..\extern\implot\implot.h ..\include\imgui\implot.h
copy ..\extern\implot\implot_internal.h ..\include\imgui\implot_internal.h

copy ..\extern\implot\implot.cpp ..\src\imgui\implot.cpp
copy ..\extern\implot\implot_items.cpp ..\src\imgui\implot_items.cpp