cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../out)
project(open_cv_web)

set(CMAKE_CXX_STANDARD 17)

set(IMGUI_SOURCES
    libs/imgui/imgui.h
    libs/imgui/backends/imgui_impl_glfw.h
    libs/imgui/backends/imgui_impl_opengl3.h
)

find_package(OpenCV REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(libs/imgui)
include_directories(libs/imgui/backends)
include_directories(/usr/include/GLFW)

add_executable(open_cv_web src/open_cv_web.cpp ${IMGUI_SOURCES})

target_link_libraries(open_cv_web PRIVATE ${OpenCV_LIBS} glfw)