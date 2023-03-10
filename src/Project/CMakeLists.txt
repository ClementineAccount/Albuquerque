cmake_minimum_required(VERSION 3.14)
project(Project)

add_custom_target(copy_data ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)   

target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)

set(sourceFiles
    Main.cpp
    ProjectApplication.cpp
)
set(headerFiles
    include/Project/ProjectApplication.hpp
)
add_executable(Project ${sourceFiles} ${headerFiles})
add_dependencies(Project copy_data)

target_include_directories(Project PRIVATE include)
target_include_directories(Project.Library PUBLIC include)
target_link_libraries(Project PRIVATE glad glfw imgui glm cgltf stb_image spdlog Project.Library)
