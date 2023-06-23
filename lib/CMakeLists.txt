add_subdirectory(extra)


include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.10.0
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

message("Fetching spdlog")
FetchContent_MakeAvailable(spdlog)

#----------------------------------------------------------------------

FetchContent_Declare(
    tracy
    GIT_REPOSITORY  https://github.com/wolfpld/tracy.git
    GIT_TAG         v0.9.1
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
)

set(TRACY_ENABLE ON CACHE BOOL "Enable profiling")
#set(TRACY_NO_SYSTEM_TRACING ON CACHE BOOL "Disable System Tracing")
set(TRACY_ONLY_IPV4 ON CACHE BOOL "IPv4 only")
option(TRACY_ENABLE "Enable profiling" ON)
#option(TRACY_NO_SYSTEM_TRACING "Disable System Tracing" ON)
option(TRACY_ONLY_IPV4 "IPv4 only" ON)
message("Fetching tracy")
FetchContent_MakeAvailable(tracy)

#----------------------------------------------------------------------

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        v1.88
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    message("Fetching imgui")
    FetchContent_Populate(imgui)

    add_library(imgui
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)

    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${glfw_SOURCE_DIR}/include)

    target_link_libraries(imgui PRIVATE glfw)
endif ()



#----------------------------------------------------------------------

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

message("Fetching glm")
FetchContent_MakeAvailable(glm)
target_compile_definitions(glm INTERFACE GLM_FORCE_SILENT_WARNINGS)

#----------------------------------------------------------------------

option(TINYGLTF_BUILD_LOADER_EXAMPLE "" OFF)
FetchContent_Declare(
    tinygltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG        544969b7324cd6bba29f6203c7d78c7ea92dbab0
)
message("Fetching tinygltf")
FetchContent_MakeAvailable(tinygltf)

FetchContent_Declare(
    cgltf
    GIT_REPOSITORY  https://github.com/jkuhlmann/cgltf.git
    GIT_TAG         master
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
)
FetchContent_GetProperties(cgltf)
if(NOT cgltf_POPULATED)
    FetchContent_Populate(cgltf)
    message("Fetching cgltf")

    add_library(cgltf INTERFACE ${cgltf_SOURCE_DIR}/cgltf.h)
    target_include_directories(cgltf INTERFACE ${cgltf_SOURCE_DIR})
endif()

#----------------------------------------------------------------------
set(FWOG_BUILD_EXAMPLES false)
FetchContent_Declare(
    fwog
    GIT_REPOSITORY https://github.com/JuanDiegoMontoya/Fwog.git
    GIT_TAG        v0.1.1
)
message("Fetching fwog")
FetchContent_MakeAvailable(fwog)

#Is this really the way? Just getting SDL straight away? Well lets see...

FetchContent_Declare(
    SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-2.26.5 
)
message("Fetching SDL")
FetchContent_GetProperties(SDL)
if(NOT SDL_POPULATED)
  FetchContent_Populate(SDL)
  add_subdirectory(${sdl_SOURCE_DIR} ${sdl_BINARY_DIR})
endif()



FetchContent_Declare(
    SDL_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG        release-2.6.3
)
message("Fetching SDL_mixer")
FetchContent_GetProperties(SDL_mixer)
if(NOT SDL_mixer_POPULATED)
  option(SDL2MIXER_INSTALL "Enable SDL2mixer install target" OFF)
  option(SDL2MIXER_DEPS_SHARED "Default value for loading dependencies dynamically" OFF)
  option(SDL2MIXER_SAMPLES "Build the SDL2_mixer sample program(s)" OFF)
  option(SDL2MIXER_CMD "Support an external music player" OFF)
  option(SDL2MIXER_FLAC "Enable FLAC music" OFF)
  option(SDL2MIXER_MOD "Support loading MOD music" OFF)
  option(SDL2MIXER_MP3 "Enable MP3 music" OFF)
  option(SDL2MIXER_MIDI "Enable MIDI music" OFF)
  option(SDL2MIXER_OPUS "Enable Opus music" OFF)
  FetchContent_Populate(SDL_mixer)
  add_subdirectory(${sdl_mixer_SOURCE_DIR} ${sdl_mixer_BINARY_DIR})
endif()





#----------------------------------------------------------------------

FetchContent_Declare(
    stb_image
    GIT_REPOSITORY  https://github.com/nothings/stb.git
    GIT_TAG         master
    GIT_SHALLOW     TRUE
    GIT_PROGRESS    TRUE
)
FetchContent_GetProperties(stb_image)
if(NOT stb_image_POPULATED)
    FetchContent_Populate(stb_image)
    message("Fetching stb_image")

    add_library(stb_image INTERFACE ${stb_image_SOURCE_DIR}/stb_image.h)
    target_include_directories(stb_image INTERFACE ${stb_image_SOURCE_DIR})
endif()
