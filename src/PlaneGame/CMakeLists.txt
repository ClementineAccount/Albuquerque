cmake_minimum_required(VERSION 3.15)
project(PlaneGame)
add_custom_target(copy_data ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data)   

include(FetchContent)

target_include_directories(imgui PUBLIC include)
target_include_directories(spdlog PUBLIC include)
target_include_directories(fwog PUBLIC include)

#message("(Plane Game) add_library soloud")
#add_library(soloud STATIC IMPORTED)
#set_target_properties(soloud PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/soloud_static.lib)
#set_target_properties(soloud PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/soloud/include)


set(sourceFiles
    Main.cpp
    ConfigReader.cpp
    TestRunner.cpp
    SceneLoader.cpp
    ProjectApplication.cpp
)

set(headerFiles
    include/SceneLoader.h
    include/ConfigReader.h
    include/TestRunner.h
    include/ProjectApplication.hpp
    )

add_executable(PlaneGame ${sourceFiles} ${headerFiles} "TestRunner.cpp")

add_dependencies(PlaneGame copy_data)


target_link_libraries(PlaneGame PRIVATE TracyClient lua54 freetype soloud glad glfw imgui glm cgltf tinygltf stb_image spdlog fwog BulletDynamics BulletCollision LinearMath Albuquerque)