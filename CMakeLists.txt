cmake_minimum_required(VERSION 3.12)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)
set(CMAKE_C_COMPILER "C:/MinGW/bin/gcc.exe")
set(CMAKE_CXX_COMPILER "C:/MinGW/bin/g++.exe")
# set(CMAKE_GENERATOR Ninja)
# set(CMAKE_MAKE_PROGRAM Ninja)

project(example_glfw_opengl3)

# Specify the path to the Emscripten toolchain file
set(CMAKE_TOOLCHAIN_FILE "$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake")

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui/)
set(GLM_DIR ${CMAKE_SOURCE_DIR}/libs/glm/)

include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(./libs)
include_directories(${GLM_DIR})

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
# find_package(assimp CONFIG REQUIRED)

# add_executable(${PROJECT_NAME} src/main.cpp)

# Link the ImGui source files
# target_sources(${PROJECT_NAME} PRIVATE
#     ${IMGUI_DIR}/imgui.cpp
#     ${IMGUI_DIR}/imgui_demo.cpp
#     ${IMGUI_DIR}/imgui_draw.cpp
#     ${IMGUI_DIR}/imgui_tables.cpp
#     ${IMGUI_DIR}/imgui_widgets.cpp
#     ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
#     ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
# )

# target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)

# target_link_libraries(${PROJECT_NAME} OpenGL::GL glfw GLEW::GLEW)

add_executable(test src/test.cpp)

# Link the ImGui source files
target_sources(test PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(test PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLM_DIR}
    )

target_link_libraries(test
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

# Docking example
add_executable(docking src/docking.cpp)

target_sources(docking PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${GLM_DIR}/glm/glm.hpp
    src/mesh.cpp
    src/mesh.hpp
    src/quaternion.cpp
    src/quaternion.hpp
    src/transform.cpp
    src/transform.hpp
    src/PointLight.cpp
    src/PointLight.hpp
    src/camera.cpp
    src/camera.hpp
    src/renderer.cpp
    src/renderer.hpp
    libs/stl.h
)

target_include_directories(docking PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLM_DIR}
    )

target_link_libraries(docking
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)