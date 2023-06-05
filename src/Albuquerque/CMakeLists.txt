cmake_minimum_required(VERSION 3.14)
project(Albuquerque)

add_subdirectory(lib)

set(sourceFiles
    Application.cpp
)

set(headerFiles
    include/Albuquerque/Application.hpp
)

add_library(Albuquerque ${sourceFiles} ${headerFiles})

find_package(OpenGL REQUIRED)

target_include_directories(glfw PUBLIC include)
target_include_directories(glad PUBLIC include)
target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)

target_include_directories(Albuquerque PRIVATE include)

#To Do: Make it properly use CMake for the soloud.lib or something
#target_link_libraries(Project.Library PRIVATE glfw glad glm TracyClient spdlog imgui fwog)
target_link_libraries(Albuquerque PRIVATE glfw glad glm TracyClient spdlog imgui fwog)


