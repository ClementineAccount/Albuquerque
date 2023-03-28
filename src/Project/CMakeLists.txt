cmake_minimum_required(VERSION 3.15)
project(Project)
add_custom_target(copy_data ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)   

include(FetchContent)



target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)
target_include_directories(fwog PUBLIC include)

message("(Project) add_library soloud")
add_library(soloud STATIC IMPORTED)
set_target_properties(soloud PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/soloud_static.lib)
set_target_properties(soloud PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/include)

message("(Project) add_library freetype")
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/freetype/freetype.lib)
set_target_properties(freetype PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/freetype/include)



message("(Project) add_library lua")
add_library(lua54 STATIC IMPORTED)
set_target_properties(lua54 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/lua/lua54.lib)
set_target_properties(lua54 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/lua/include)



set(sourceFiles
    Main.cpp
    SceneLoader.cpp
    ProjectApplication.cpp
    MeshLoading.cpp
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

target_link_libraries(Project PRIVATE lua54 freetype soloud glad glfw imgui glm cgltf tinygltf stb_image spdlog fwog Project.Library)
target_link_libraries(Project PRIVATE Jolt)