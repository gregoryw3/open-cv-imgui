cmake_minimum_required(VERSION 3.12)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)

# Add ImGui library
add_subdirectory(path/to/imgui)

# Add your source files
add_executable(MyProject main.cpp)

# Link ImGui library
target_link_libraries(MyProject PRIVATE imgui)