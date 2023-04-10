cmake_minimum_required(VERSION 3.15)
project(Project)
add_custom_target(copy_data ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)   

include(FetchContent)



target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)
target_include_directories(fwog PUBLIC include)

add_library(soloud STATIC IMPORTED)
set_target_properties(soloud PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/soloud_static.lib)
set_target_properties(soloud PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/include)

set(sourceFiles
    Main.cpp
    SceneLoader.cpp
    ProjectApplication.cpp
)

set(headerFiles
    include/SceneLoader.h
    include/ProjectApplication.hpp
    )

add_executable(Project ${sourceFiles} ${headerFiles})
add_dependencies(Project copy_data)


target_include_directories(Project PRIVATE ${JoltPhysics_SOURCE_DIR}/.. include)
target_include_directories(Project.Library PUBLIC ${JoltPhysics_SOURCE_DIR}/.. include)

target_link_libraries(Project PRIVATE soloud glad glfw imgui glm cgltf tinygltf stb_image spdlog fwog Project.Library)
target_link_libraries(Project PRIVATE Jolt)