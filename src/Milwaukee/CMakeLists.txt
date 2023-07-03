cmake_minimum_required(VERSION 3.14)
project(Milwaukee)

add_custom_target(copy_data_milwaukee ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)   

set(sourceFiles
    Main.cpp
    MilwaukeeApplication.cpp
)

add_executable(Milwaukee ${sourceFiles})
add_dependencies(Milwaukee copy_data_milwaukee)

target_include_directories(Milwaukee PRIVATE include)

target_link_libraries(Milwaukee PRIVATE TracyClient glad glfw imgui glm cgltf stb_image spdlog Albuquerque)
