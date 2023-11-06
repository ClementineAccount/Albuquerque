cmake_minimum_required(VERSION 3.14)
project(HelloSDL)

#Could also consider 'glob'-ing it here to prevent a FetchContent call
set(sourceFiles
    main2.cpp
)

add_library(SDL2 STATIC IMPORTED)
set_target_properties(SDL2 PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/SDL2.lib)
set_target_properties(SDL2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/include)

add_library(SDL2_Mixer STATIC IMPORTED)
set_target_properties(SDL2_Mixer PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/SDL2_Mixer.lib)
set_target_properties(SDL2_Mixer PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/include)

add_library(SDL2_image STATIC IMPORTED)
set_target_properties(SDL2_image PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/SDL2_image.lib)
set_target_properties(SDL2_image PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/SDL2/include)



target_include_directories(glad PUBLIC include)

add_executable(HelloSDL ${sourceFiles})
target_include_directories(HelloSDL PRIVATE include)
target_link_libraries(HelloSDL PRIVATE glad opengl32 glu32 SDL2 SDL2_Mixer)
