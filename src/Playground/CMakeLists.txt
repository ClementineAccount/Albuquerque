cmake_minimum_required(VERSION 3.14)
project(Playground)

add_custom_target(copy_playground_data ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)   

#Could also consider 'glob'-ing it here to prevent a FetchContent call
set(sourceFiles
    Main.cpp
    PlaygroundApplication.cpp
)

add_executable(Playground ${sourceFiles})
add_dependencies(Playground copy_playground_data)

target_include_directories(Playground PRIVATE include)

target_link_libraries(Playground PRIVATE fwog glad glfw imgui glm cgltf stb_image spdlog Albuquerque)
