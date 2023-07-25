cmake_minimum_required(VERSION 3.14)
project(Albuquerque)

add_subdirectory(lib)

set(sourceFiles
    Camera.cpp
    Application.cpp
)

set(headerFiles
    include/Albuquerque/Application.hpp
    include/Albuquerque/Camera.hpp
    include/Albuquerque/DrawObject.hpp
    include/Albuquerque/Primitives.hpp
)

add_library(Albuquerque ${sourceFiles} ${headerFiles})

find_package(OpenGL REQUIRED)


#Need to check as the current Deccer CMake Template have improved upon target_include here
target_include_directories(glfw PUBLIC include)
target_include_directories(glad PUBLIC include)
target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)

target_include_directories(Albuquerque PRIVATE include)

#target_link_libraries(Project.Library PRIVATE glfw glad glm TracyClient spdlog imgui fwog)
target_link_libraries(Albuquerque PRIVATE glfw glad glm TracyClient spdlog imgui fwog)


