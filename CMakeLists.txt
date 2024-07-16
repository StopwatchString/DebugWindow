cmake_minimum_required(VERSION 3.27)

project(          DebugWindow VERSION 0.1
                  DESCRIPTION "DebugWindow"
                  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE INCLUDES "include/*.h*")

source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${SOURCES})
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${INCLUDES})

add_executable(${PROJECT_NAME} ${SOURCES} ${INCLUDES})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} 
        PRIVATE
         "${PROJECT_SOURCE_DIR}/include"
         "${PROJECT_SOURCE_DIR}/include/imgui")

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)