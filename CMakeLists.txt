cmake_minimum_required(VERSION 3.27)

project(          DebugWindow VERSION 0.2
                  DESCRIPTION "DebugWindow"
                  LANGUAGES CXX)

if(MSVC)
    add_compile_options(/MP)
endif()

set(CMAKE_CXX_STANDARD 14)

set(DEBUGWINDOWWIN32_SOURCES
    "src/main.cpp"
    "src/DebugWindow/DebugWindow.cpp"
    "src/DebugWindow/DebugWindowWin32.cpp"

    "src/DebugWindow/imgui.cpp"
    "src/DebugWindow/imgui_demo.cpp"
    "src/DebugWindow/imgui_draw.cpp"
    "src/DebugWindow/imgui_impl_opengl3.cpp"
    "src/DebugWindow/imgui_impl_win32.cpp"
    "src/DebugWindow/imgui_tables.cpp"
    "src/DebugWindow/imgui_widgets.cpp"

    "src/DebugWindow/implot.cpp"
    "src/DebugWindow/implot_items.cpp"
)
set(DEBUGWINDOWWIN32_INCLUDES
    "include/DebugWindow/DebugWindow.h"
    "include/DebugWindow/DebugWindowWin32.h"

    "include/DebugWindow/imconfig.h"
    "include/DebugWindow/imgui.h"
    "include/DebugWindow/imgui_impl_opengl3.h"
    "include/DebugWindow/imgui_impl_opengl3_loader.h"
    "include/DebugWindow/imgui_impl_win32.h"
    "include/DebugWindow/imgui_internal.h"
    "include/DebugWindow/imstb_rectpack.h"
    "include/DebugWindow/imstb_textedit.h"
    "include/DebugWindow/imstb_truetype.h"

    "include/DebugWindow/implot.h"
    "include/DebugWindow/implot_internal.h"
)

# Provides heirarchical filters in Visual Studio
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${DEBUGWINDOWWIN32_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${DEBUGWINDOWWIN32_INCLUDES})

add_executable("DebugWindowWin32" WIN32 ${DEBUGWINDOWWIN32_SOURCES} ${DEBUGWINDOWWIN32_INCLUDES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "DebugWindowWin32")

# Set Console subsystem for Debug configuration
set_target_properties("DebugWindowWin32" PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

target_include_directories("DebugWindowWin32"
        PRIVATE
         "${PROJECT_SOURCE_DIR}/include"
         "${PROJECT_SOURCE_DIR}/include/DebugWindow")

target_compile_definitions("DebugWindowWin32" PRIVATE
    DEBUGWINDOW_WIN32
)

find_package(OpenGL REQUIRED)
target_link_libraries("DebugWindowWin32" PRIVATE OpenGL::GL)

#############################################################################

set(DEBUGWINDOWGLFW_SOURCES
    "src/main.cpp"
    "src/DebugWindow/DebugWindow.cpp"
    "src/DebugWindow/DebugWindowGLFW.cpp"

    "src/DebugWindow/imgui.cpp"
    "src/DebugWindow/imgui_demo.cpp"
    "src/DebugWindow/imgui_draw.cpp"
    "src/DebugWindow/imgui_impl_opengl3.cpp"
    "src/DebugWindow/imgui_impl_glfw.cpp"
    "src/DebugWindow/imgui_tables.cpp"
    "src/DebugWindow/imgui_widgets.cpp"

    "src/DebugWindow/implot.cpp"
    "src/DebugWindow/implot_items.cpp"
)
set(DEBUGWINDOWGLFW_INCLUDES
    "include/DebugWindow/DebugWindow.h"
    "include/DebugWindow/DebugWindowGLFW.h"

    "include/DebugWindow/imconfig.h"
    "include/DebugWindow/imgui.h"
    "include/DebugWindow/imgui_impl_opengl3.h"
    "include/DebugWindow/imgui_impl_opengl3_loader.h"
    "include/DebugWindow/imgui_impl_glfw.h"
    "include/DebugWindow/imgui_internal.h"
    "include/DebugWindow/imstb_rectpack.h"
    "include/DebugWindow/imstb_textedit.h"
    "include/DebugWindow/imstb_truetype.h"

    "include/DebugWindow/implot.h"
    "include/DebugWindow/implot_internal.h"

    "include/GLFW/glfw3.h"
)

# Provides heirarchical filters in Visual Studio
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${DEBUGWINDOWGLFW_SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${DEBUGWINDOWGLFW_INCLUDES})

add_executable("DebugWindowGLFW" WIN32 ${DEBUGWINDOWGLFW_SOURCES} ${DEBUGWINDOWGLFW_INCLUDES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "DebugWindowGLFW")

# Set Console subsystem for Debug configuration
set_target_properties("DebugWindowGLFW" PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)

target_include_directories("DebugWindowGLFW"
        PRIVATE
         "${PROJECT_SOURCE_DIR}/include"
         "${PROJECT_SOURCE_DIR}/include/DebugWindow")

target_compile_definitions("DebugWindowGLFW" PRIVATE
    DEBUGWINDOW_GLFW
)

find_package(OpenGL REQUIRED)
target_link_libraries("DebugWindowGLFW" PRIVATE OpenGL::GL)

# Add the directory containing glfw3.lib to the link directories
target_link_directories("DebugWindowGLFW" PRIVATE "${PROJECT_SOURCE_DIR}/lib")

# Link glfw3.lib to the target
target_link_libraries("DebugWindowGLFW" PRIVATE glfw3)