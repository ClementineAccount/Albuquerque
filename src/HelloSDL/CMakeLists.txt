cmake_minimum_required(VERSION 3.14)
project(HelloSDL)

#Could also consider 'glob'-ing it here to prevent a FetchContent call
set(sourceFiles
    main.cpp
)

add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/SDL2.lib)
set_target_properties(SDL2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/include)

add_executable(HelloSDL ${sourceFiles})
target_include_directories(HelloSDL PRIVATE include)
target_link_libraries(HelloSDL PRIVATE SDL2)
