cmake_minimum_required(VERSION 3.15)
project(PlaneGame)
add_custom_target(copy_data ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)   

include(FetchContent)

target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)
target_include_directories(fwog PUBLIC include)

message("(Plane Game) add_library soloud")
add_library(soloud STATIC IMPORTED)
set_target_properties(soloud PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/soloud_static.lib)
set_target_properties(soloud PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/include)

message("(Plane Game) add_library freetype")
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/freetype/freetype.lib)
set_target_properties(freetype PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/freetype/include)

message("(Plane Game) add_library lua")
add_library(lua54 STATIC IMPORTED)
set_target_properties(lua54 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/lua/lua54.lib)
set_target_properties(lua54 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/lua/include)

message("(Plane Game) add_library BulletDynamics")
add_library(BulletDynamics STATIC IMPORTED)
set_target_properties(BulletDynamics PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/bullet/BulletDynamics.lib)
set_target_properties(BulletDynamics PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/bullet/include)

message("(Plane Game) add_library BulletCollision")
add_library(BulletCollision STATIC IMPORTED)
set_target_properties(BulletCollision PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/bullet/BulletCollision.lib)
set_target_properties(BulletCollision PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/bullet/include)

message("(Plane Game) add_library LinearMath (Bullet)")
add_library(LinearMath STATIC IMPORTED)
set_target_properties(LinearMath PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/bullet/LinearMath.lib)
set_target_properties(LinearMath PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/bullet/include)



set(sourceFiles
    Main.cpp
    SceneLoader.cpp
    ImGuizmo.cpp
    ProjectApplication.cpp
)

set(headerFiles
    include/SceneLoader.h
    include/ImGuizmo.h
    include/ProjectApplication.hpp
    )

add_executable(PlaneGame ${sourceFiles} ${headerFiles})

add_dependencies(PlaneGame copy_data)


target_link_libraries(PlaneGame PRIVATE TracyClient lua54 freetype soloud glad glfw imgui glm cgltf tinygltf stb_image spdlog fwog BulletDynamics BulletCollision LinearMath Albuquerque)