cmake_minimum_required(VERSION 3.14)
project(ECSDemo)

#Could also consider 'glob'-ing it here to prevent a FetchContent call
set(sourceFiles
    main.cpp
)

add_executable(ECSDemo ${sourceFiles})
target_include_directories(ECSDemo PRIVATE include)
target_link_libraries(ECSDemo PRIVATE fwog glad glfw imgui glm cgltf stb_image spdlog Albuquerque)
