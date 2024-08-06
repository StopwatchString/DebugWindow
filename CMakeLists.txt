cmake_minimum_required(VERSION 3.27)

project(          DebugWindow VERSION 0.2
                  DESCRIPTION "DebugWindow"
                  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

set(DEBUGWINDOWWIN32_SOURCES
    "src/DebugWindow.cpp"
    "src/DebugWindowWin32.cpp"
    "src/debugWindowWin32main.cpp"

    "src/imgui/imgui.cpp"
    "src/imgui/imgui_demo.cpp"
    "src/imgui/imgui_draw.cpp"
    "src/imgui/imgui_impl_opengl3.cpp"
    "src/imgui/imgui_impl_win32.cpp"
    "src/imgui/imgui_tables.cpp"
    "src/imgui/imgui_widgets.cpp"

    "src/imgui/implot.cpp"
    "src/imgui/implot_items.cpp"
)
set(DEBUGWINDOWWIN32_INCLUDES
    "include/DebugWindow.h"
    "include/DebugWindowWin32.h"

    "include/imgui/imconfig.h"
    "include/imgui/imgui.h"
    "include/imgui/imgui_impl_opengl3.h"
    "include/imgui/imgui_impl_opengl3_loader.h"
    "include/imgui/imgui_impl_win32.h"
    "include/imgui/imgui_internal.h"
    "include/imgui/imstb_rectpack.h"
    "include/imgui/imstb_textedit.h"
    "include/imgui/imstb_truetype.h"

    "include/imgui/implot.h"
    "include/imgui/implot_internal.h"
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
         "${PROJECT_SOURCE_DIR}/include/imgui")

find_package(OpenGL REQUIRED)
target_link_libraries("DebugWindowWin32" PRIVATE OpenGL::GL)

#############################################################################

set(DEBUGWINDOWGLFW_SOURCES
    "src/DebugWindow.cpp"
    "src/DebugWindowGLFW.cpp"
    "src/debugWindowGLFWmain.cpp"

    "src/imgui/imgui.cpp"
    "src/imgui/imgui_demo.cpp"
    "src/imgui/imgui_draw.cpp"
    "src/imgui/imgui_impl_opengl3.cpp"
    "src/imgui/imgui_impl_glfw.cpp"
    "src/imgui/imgui_tables.cpp"
    "src/imgui/imgui_widgets.cpp"

    "src/imgui/implot.cpp"
    "src/imgui/implot_items.cpp"
)
set(DEBUGWINDOWGLFW_INCLUDES
    "include/DebugWindow.h"
    "include/DebugWindowGLFW.h"

    "include/imgui/imconfig.h"
    "include/imgui/imgui.h"
    "include/imgui/imgui_impl_opengl3.h"
    "include/imgui/imgui_impl_opengl3_loader.h"
    "include/imgui/imgui_impl_glfw.h"
    "include/imgui/imgui_internal.h"
    "include/imgui/imstb_rectpack.h"
    "include/imgui/imstb_textedit.h"
    "include/imgui/imstb_truetype.h"

    "include/imgui/implot.h"
    "include/imgui/implot_internal.h"

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
         "${PROJECT_SOURCE_DIR}/include/imgui")

find_package(OpenGL REQUIRED)
target_link_libraries("DebugWindowGLFW" PRIVATE OpenGL::GL)

# Add the directory containing glfw3.lib to the link directories
target_link_directories("DebugWindowGLFW" PRIVATE "${PROJECT_SOURCE_DIR}/lib")

# Link glfw3.lib to the target
target_link_libraries("DebugWindowGLFW" PRIVATE glfw3)