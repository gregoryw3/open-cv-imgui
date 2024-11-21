cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)
project(example_glfw_opengl3)

# Specify the path to the Emscripten toolchain file
set(CMAKE_TOOLCHAIN_FILE "$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake")

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui/)

include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(./libs)

add_executable(${PROJECT_NAME} src/main.cpp)

#set(SOURCES
#        src/main.cpp
#        ${IMGUI_DIR}/imgui.cpp
#        ${IMGUI_DIR}/imgui_demo.cpp
#        ${IMGUI_DIR}/imgui_draw.cpp
#        ${IMGUI_DIR}/imgui_tables.cpp
#        ${IMGUI_DIR}/imgui_widgets.cpp
#        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
#        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
#)

# Link the ImGui source files
target_sources(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)



target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL glfw)

# target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw)