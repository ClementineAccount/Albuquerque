cmake_minimum_required(VERSION 3.15)
project(CarGame)
add_custom_target(copy_data_cargame ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data_cargame ${CMAKE_CURRENT_BINARY_DIR}/data_cargame)   

include(FetchContent)

target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)
target_include_directories(fwog PUBLIC include)

message("(CarGame) add_library soloud")
add_library(soloud STATIC IMPORTED)
set_target_properties(soloud PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/soloud_static.lib)
set_target_properties(soloud PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/include)

message("(CarGame) add_library freetype")
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/freetype/freetype.lib)
set_target_properties(freetype PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/freetype/include)

message("(CarGame) add_library lua")
add_library(lua54 STATIC IMPORTED)
set_target_properties(lua54 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/lua/lua54.lib)
set_target_properties(lua54 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/lua/include)



set(sourceFiles
    Main.cpp
    SceneLoader.cpp
    CarApplication.cpp
)

set(headerFiles
    include/CarGame/CarApplication.hpp
    include/CarGame/SceneLoader.h
    )

add_executable(CarGame ${sourceFiles} ${headerFiles})
add_dependencies(CarGame copy_data_cargame)


target_link_libraries(CarGame PRIVATE TracyClient lua54 freetype soloud glad glfw imgui glm cgltf tinygltf stb_image spdlog fwog Albuquerque)