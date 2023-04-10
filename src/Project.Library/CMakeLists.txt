cmake_minimum_required(VERSION 3.14)
project(Project.Library)

add_subdirectory(lib)

set(sourceFiles
    Application.cpp
)

set(headerFiles
    include/Project.Library/Application.hpp
)

add_library(Project.Library ${sourceFiles} ${headerFiles})

find_package(OpenGL REQUIRED)

target_include_directories(glfw PUBLIC include)
target_include_directories(glad PUBLIC include)
target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)

add_library(soloud STATIC IMPORTED)
set_target_properties(soloud PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/soloud_static.lib)
set_target_properties(soloud PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/include)

target_include_directories(Project.Library PRIVATE include)

#To Do: Make it properly use CMake for the soloud.lib or something
#target_link_libraries(Project.Library PRIVATE glfw glad glm TracyClient spdlog imgui fwog)
target_link_libraries(Project.Library PRIVATE soloud glfw glad glm TracyClient spdlog imgui fwog)


